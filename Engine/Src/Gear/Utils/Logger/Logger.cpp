#include<Gear/Utils/Logger/Logger.h>

#include<Gear/Utils/Internal/LoggerInternal.h>

#include<fileapi.h>

#include<consoleapi.h>

#include<string>

#include<queue>

namespace Gear::Utils::Logger
{
	class LoggerImpl
	{
	public:

		LoggerImpl(const LoggerImpl&) = delete;

		void operator=(const LoggerImpl&) = delete;

		LoggerImpl() :
			isRunning(true), fileHandle(nullptr), consoleHandle(nullptr)
		{
			fileHandle = CreateFileW(File::UserDataPath(L"log.txt").cstr(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				fileHandle = nullptr;
			}

			consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

			if (consoleHandle == INVALID_HANDLE_VALUE)
			{
				consoleHandle = nullptr;
			}

			if (consoleHandle)
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

			if (fileHandle)
			{
				CloseHandle(fileHandle);

				fileHandle = nullptr;
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

		std::queue<LogMessage> messages;

		bool isRunning;

		std::mutex queueLock;

		std::condition_variable cv;

		std::string temp;

		std::thread worker;

		HANDLE fileHandle;

		HANDLE consoleHandle;

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

				message.context->readIndexIncrement();

				if (fileHandle)
				{
					DWORD bytesWritten = 0;

					WriteFile(fileHandle, temp.c_str(), static_cast<DWORD>(temp.size()), &bytesWritten, nullptr);
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

					message.context->readIndexIncrement();

					//ERROR类型在Gear::failureExit处被统一处理
					if (consoleHandle && message.type != LogType::LOG_ERROR)
					{
						WriteConsoleA(consoleHandle, temp.c_str(), static_cast<DWORD>(temp.size()), nullptr, nullptr);
					}

					if (fileHandle)
					{
						DWORD bytesWritten = 0;

						WriteFile(fileHandle, temp.c_str(), static_cast<DWORD>(temp.size()), &bytesWritten, nullptr);
					}

					lock.lock();
				}
			}
		}
	};

	UniquePtr<LoggerImpl> impl;

	namespace Internal
	{
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
		impl->submitLogMessage(msg);
	}
}
