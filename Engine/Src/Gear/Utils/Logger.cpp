#include<Gear/Utils/Logger.h>

#include<Gear/Utils/Internal/LoggerInternal.h>

#include<iostream>

#include<fstream>

#include<queue>

namespace Gear::Utils::Logger
{
	namespace Internal
	{
		class LoggerImpl
		{
		public:

			LoggerImpl(const LoggerImpl&) = delete;

			void operator=(const LoggerImpl&) = delete;

			LoggerImpl() :
				isRunning(true)
			{
				//设置locale为.UTF-8用于多语言支持
				std::locale::global(std::locale(".UTF-8"));

				file = std::ofstream("log.txt", std::ios_base::out | std::ios_base::trunc);

				consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

				consoleEnabled = (INVALID_HANDLE_VALUE != consoleHandle);

				if (consoleEnabled)
				{
					SetConsoleOutputCP(CP_UTF8);

					DWORD consoleMode = 0;

					GetConsoleMode(consoleHandle, &consoleMode);

					consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

					SetConsoleMode(consoleHandle, consoleMode);
				}

				worker = std::thread(&LoggerImpl::workerLoop, this);
			}

			~LoggerImpl()
			{
				shutdown();

				if (file.is_open())
				{
					file.flush();

					file.close();
				}
			}

			void submitLogMessage(const LogMessage& msg)
			{
				{
					std::lock_guard<std::mutex> lockGuard(queueLock);

					messages.push(msg);
				}

				cv.notify_one();
			}

		private:

			std::ofstream file;

			std::queue<LogMessage> messages;

			bool isRunning;

			std::mutex queueLock;

			std::condition_variable cv;

			std::string temp;

			std::thread worker;

			HANDLE consoleHandle;

			bool consoleEnabled;

			void shutdown()
			{
				{
					std::lock_guard<std::mutex> lockGuard(queueLock);

					isRunning = false;
				}

				cv.notify_one();

				if (worker.joinable())
				{
					worker.join();
				}

				while (!messages.empty())
				{
					LogMessage message = std::move(messages.front());

					messages.pop();

					temp = message.str;

					{
						std::lock_guard<std::mutex> inUseLock(message.inUseMutex);

						message.readIndex++;
					}

					message.inUseCV.notify_one();

					if (file.is_open())
					{
						file << temp;
					}
				}
			}

			void workerLoop()
			{
				while (true)
				{
					std::unique_lock<std::mutex> lock(queueLock);

					cv.wait(lock, [this]() {return !messages.empty() || !isRunning; });

					if (!isRunning)
					{
						break;
					}

					while (!messages.empty())
					{
						LogMessage message = std::move(messages.front());

						messages.pop();

						lock.unlock();

						temp = message.str;

						{
							std::lock_guard<std::mutex> inUseLock(message.inUseMutex);

							message.readIndex++;
						}

						message.inUseCV.notify_one();

						if (consoleEnabled && message.type != LogType::LOG_ERROR)
						{
							WriteConsoleA(consoleHandle, temp.c_str(), static_cast<DWORD>(temp.size()), nullptr, nullptr);
						}

						file << temp;

						lock.lock();
					}
				}
			}
		};

		UniquePtr<LoggerImpl> impl;

		void initialize()
		{
			impl = makeUnique<LoggerImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	void submitLogMessage(const LogMessage& msg)
	{
		Internal::impl->submitLogMessage(msg);
	}
}
