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

				file = std::wofstream(L"log.txt", std::ios_base::out | std::ios_base::trunc);

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

			std::wofstream file;

			std::queue<LogMessage> messages;

			bool isRunning;

			std::mutex queueLock;

			std::condition_variable cv;

			std::wstring temp;

			std::thread worker;

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
						file << temp << L"\n";
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

						if (message.type != LogType::LOG_ERROR)
						{
							std::wcout << temp << L"\n";
						}

						file << temp << L"\n";

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