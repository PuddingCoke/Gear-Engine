#include<Gear/Utils/Logger/LogContext.h>

#include<Gear/Utils/String.h>

#include<Gear/Utils/Math.h>

namespace Gear::Utils::Logger
{
	LogContext::LogContext() :
		textColor{ "" },
		displayColor{ "" },
		writeIndex(0),
		readIndex(0),
		messageStr(nullptr)
	{
		resetState();
	}

	LogContext::~LogContext()
	{
		std::unique_lock<std::mutex> readIndexLock(readIndexMutex);

		readIndexCV.wait(readIndexLock, [this]() { return writeIndex == readIndex; });
	}

	LogContext& LogContext::get()
	{
		thread_local UniquePtr<LogContext> context = makeUnique<LogContext>();

		return *context;
	}

	void LogContext::readIndexIncrement()
	{
		{
			std::lock_guard<std::mutex> readIndexLock(readIndexMutex);

			readIndex++;
		}

		readIndexCV.notify_one();
	}

	void LogContext::createLogMessage(const std::string_view& functionName, const LogType& type)
	{
		resetState();

		acquireReusableSlot();

		packHeader(functionName, type);

		packArgument(LogColor::defaultColor);
	}

	void LogContext::packArgument(const std::wstring& arg)
	{
		setDisplayColor(textColor);

		const int32_t sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, arg.data(), -1, nullptr, 0, nullptr, nullptr);

		if (sizeNeeded == 0)
		{
			*messageStr += "WideCharToMultiByte调用失败";
		}
		else if (sizeNeeded <= convertBufferLength)
		{
			WideCharToMultiByte(CP_UTF8, 0, arg.data(), -1, &convertBuffer[0], sizeNeeded, nullptr, nullptr);

			*messageStr += convertBuffer;
		}
		else
		{
			*messageStr += String::wStringToString(arg);
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const wchar_t* arg)
	{
		setDisplayColor(textColor);

		const int32_t sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, arg, -1, nullptr, 0, nullptr, nullptr);

		if (sizeNeeded == 0)
		{
			*messageStr += "WideCharToMultiByte调用失败";
		}
		else if (sizeNeeded <= convertBufferLength)
		{
			WideCharToMultiByte(CP_UTF8, 0, arg, -1, &convertBuffer[0], sizeNeeded, nullptr, nullptr);

			*messageStr += convertBuffer;
		}
		else
		{
			*messageStr += String::wStringToString(arg);
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const std::string& arg)
	{
		setDisplayColor(textColor);

		*messageStr += arg;

		*messageStr += " ";
	}

	void LogContext::packArgument(const char* arg)
	{
		setDisplayColor(textColor);

		*messageStr += arg;

		*messageStr += " ";
	}

	void LogContext::packArgument(const std::u8string& arg)
	{
		setDisplayColor(textColor);

		*messageStr += reinterpret_cast<const char*>(arg.c_str());

		*messageStr += " ";
	}

	void LogContext::packArgument(const char8_t* arg)
	{
		setDisplayColor(textColor);

		*messageStr += reinterpret_cast<const char*>(arg);

		*messageStr += " ";
	}

	void LogContext::packArgument(const bool& arg)
	{
		if (arg)
		{
			setDisplayColor(LogColor::brightGreen);

			*messageStr += "TRUE ";
		}
		else
		{
			setDisplayColor(LogColor::brightRed);

			*messageStr += "FALSE ";
		}
	}

	void LogContext::packArgument(const int32_t& arg)
	{
		setDisplayColor(LogColor::numericColor);

		if (integerMode == LogIntegerMode::HEX)
		{
			_itoa_s(arg, convertBuffer + 2, convertBufferLength - 2ull, 16);

			_strupr_s(convertBuffer + 2, convertBufferLength - 2ull);

			convertBuffer[0] = '0';

			convertBuffer[1] = 'x';

			*messageStr += convertBuffer;
		}
		else
		{
			_itoa_s(arg, convertBuffer, convertBufferLength, 10);

			*messageStr += convertBuffer;
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const int64_t& arg)
	{
		setDisplayColor(LogColor::numericColor);

		if (integerMode == LogIntegerMode::HEX)
		{
			_i64toa_s(arg, convertBuffer + 2, convertBufferLength - 2ull, 16);

			_strupr_s(convertBuffer + 2, convertBufferLength - 2ull);

			convertBuffer[0] = '0';

			convertBuffer[1] = 'x';

			*messageStr += convertBuffer;
		}
		else
		{
			_i64toa_s(arg, convertBuffer, convertBufferLength, 10);

			*messageStr += convertBuffer;
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const uint32_t& arg)
	{
		setDisplayColor(LogColor::numericColor);

		if (integerMode == LogIntegerMode::HEX)
		{
			_ultoa_s(arg, convertBuffer + 2, convertBufferLength - 2ull, 16);

			_strupr_s(convertBuffer + 2, convertBufferLength - 2ull);

			convertBuffer[0] = '0';

			convertBuffer[1] = 'x';

			*messageStr += convertBuffer;
		}
		else
		{
			_ultoa_s(arg, convertBuffer, convertBufferLength, 10);

			*messageStr += convertBuffer;
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const uint64_t& arg)
	{
		setDisplayColor(LogColor::numericColor);

		if (integerMode == LogIntegerMode::HEX)
		{
			_ui64toa_s(arg, convertBuffer + 2, convertBufferLength - 2ull, 16);

			_strupr_s(convertBuffer + 2, convertBufferLength - 2ull);

			convertBuffer[0] = '0';

			convertBuffer[1] = 'x';

			*messageStr += convertBuffer;
		}
		else
		{
			_ui64toa_s(arg, convertBuffer, convertBufferLength, 10);

			*messageStr += convertBuffer;
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const float_t& arg)
	{
		packFloatPoint(arg);
	}

	void LogContext::packArgument(const double_t& arg)
	{
		packFloatPoint(arg);
	}

	void LogContext::packArgument(const File::Path& arg)
	{
		setDisplayColor(LogColor::brightBlue);

		const std::wstring& str = arg.str();

		const int32_t sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, nullptr, 0, nullptr, nullptr);

		if (sizeNeeded == 0)
		{
			*messageStr += "WideCharToMultiByte调用失败";
		}
		else if (sizeNeeded <= convertBufferLength)
		{
			WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, &convertBuffer[0], sizeNeeded, nullptr, nullptr);

			*messageStr += convertBuffer;
		}
		else
		{
			*messageStr += String::wStringToString(str);
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const LogIntegerMode& arg)
	{
		integerMode = arg;
	}

	void LogContext::packArgument(const LogFloatPrecision& arg)
	{
		floatPrecision = arg;
	}

	void LogContext::packArgument(const LogColor& arg)
	{
		if (textColor != arg)
		{
			textColor = arg;
		}
	}

	void LogContext::packArgument(const LogNewLine&)
	{
		*messageStr += "\n";
	}

	LogType LogContext::getLogType() const
	{
		return logType;
	}

	LogMessage LogContext::getLogMessage()
	{
		return LogMessage{ *messageStr,this,logType };
	}

	void LogContext::resetState()
	{
		integerMode = LogIntegerMode::DEC;

		floatPrecision = 5;

		displayColor = LogColor::functionNameColor;
	}

	void LogContext::acquireReusableSlot()
	{
		writeIndex++;

		if (!(writeIndex - readIndex < slotNum))
		{
			std::unique_lock<std::mutex> readIndexLock(readIndexMutex);

			readIndexCV.wait(readIndexLock, [this]() { return writeIndex - readIndex < slotNum; });
		}

		messageStr = &slots[writeIndex % slotNum];

		messageStr->clear();
	}

	void LogContext::packHeader(const std::string_view& functionName, const LogType& type)
	{
		logType = type;

		const time_t currentTime = time(nullptr);

		tm localTime = {};

		localtime_s(&localTime, &currentTime);

		const std::thread::id id = std::this_thread::get_id();

		const uint32_t threadId = *(uint32_t*)&id;

		//headerStrLen = 5+2+8+1+5+2+1+10+1+5+2+length(functionName)+1+1
		//			   = 44+length(functionName)
		sprintf_s(convertBuffer, convertBufferLength, "%s[%d:%d:%d] %s{T%u} %s(%s) ", LogColor::timeStampColor.code, localTime.tm_hour, localTime.tm_min, localTime.tm_sec,
			LogColor::threadIdColor.code, threadId, LogColor::functionNameColor.code, functionName.data());

		*messageStr += convertBuffer;

		switch (type)
		{
		case LogType::LOG_SUCCESS:

			packArgument(LogColor::successColor);

			packArgument("<SUCCESS>");

			break;
		case LogType::LOG_ERROR:

			packArgument(LogColor::errorColor);

			packArgument("<ERROR>");

			break;
		case LogType::LOG_ENGINE:

			packArgument(LogColor::engineColor);

			packArgument("<ENGINE>");

			break;
		case LogType::LOG_USER:

			packArgument(LogColor::userColor);

			packArgument("<USER>");

			break;
		}
	}

	void LogContext::setDisplayColor(const LogColor& color)
	{
		if (displayColor != color)
		{
			displayColor = color;

			*messageStr += displayColor.code;
		}
	}
}
