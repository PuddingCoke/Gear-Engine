#include<Gear/Utils/Logger/LogContext.h>

#include<Gear/Utils/String.h>

namespace Gear::Utils::Logger
{
	LogContext::FloatPrecision::FloatPrecision(const int32_t precision) :
		precision(std::max(std::min(precision, 16), 0))
	{
	}

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
		std::unique_lock<std::mutex> inUseLock(inUseMutex);

		inUseCV.wait(inUseLock, [this]() { return writeIndex == readIndex; });
	}

	void LogContext::packRestArgument()
	{
	}

	void LogContext::packArgument(const std::wstring& arg)
	{
		setDisplayColor(textColor);

		const int32_t sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, arg.data(), -1, nullptr, 0, nullptr, nullptr);

		if (0 == sizeNeeded)
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
			*messageStr += String::wstringToString(arg);
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const wchar_t* arg)
	{
		setDisplayColor(textColor);

		const int32_t sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, arg, -1, nullptr, 0, nullptr, nullptr);

		if (0 == sizeNeeded)
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
			*messageStr += String::wstringToString(arg);
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

		if (integerMode == IntegerMode::HEX)
		{
			_itoa_s(arg, convertBuffer + 2, convertBufferLength - 2ull, 16);

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

		if (integerMode == IntegerMode::HEX)
		{
			_i64toa_s(arg, convertBuffer + 2, convertBufferLength - 2ull, 16);

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

		if (integerMode == IntegerMode::HEX)
		{
			_ultoa_s(arg, convertBuffer + 2, convertBufferLength - 2ull, 16);

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

		if (integerMode == IntegerMode::HEX)
		{
			_ui64toa_s(arg, convertBuffer + 2, convertBufferLength - 2ull, 16);

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

	void LogContext::packArgument(const IntegerMode& mode)
	{
		integerMode = mode;
	}

	void LogContext::packArgument(const FloatPrecision& precision)
	{
		floatPrecision = precision;
	}

	void LogContext::packArgument(const LogColor& arg)
	{
		if (textColor != arg)
		{
			textColor = arg;
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

	void LogContext::resetState()
	{
		integerMode = IntegerMode::DEC;

		floatPrecision = 5;

		displayColor = LogColor::functionNameColor;
	}
}
