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

		*messageStr += String::wstringToString(arg);

		*messageStr += " ";
	}

	void LogContext::packArgument(const wchar_t* arg)
	{
		setDisplayColor(textColor);

		*messageStr += String::wstringToString(arg);

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
			//0xFFFFFFFF\0 11
			const size_t hexMaxLength = 11ull;

			char buff[hexMaxLength] = {};

			_itoa_s(arg, buff + 2, hexMaxLength - 2ull, 16);

			buff[0] = '0';

			buff[1] = 'x';

			*messageStr += buff;
		}
		else
		{
			//-2147483648\0 12
			const size_t decMaxLength = 12ull;

			char buff[decMaxLength] = {};

			_itoa_s(arg, buff, decMaxLength, 10);

			*messageStr += buff;
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const int64_t& arg)
	{
		setDisplayColor(LogColor::numericColor);

		if (integerMode == IntegerMode::HEX)
		{
			//0xFFFFFFFFFFFFFFFF\0 19
			const size_t hexMaxLength = 19ull;

			char buff[hexMaxLength] = {};

			_i64toa_s(arg, buff + 2, hexMaxLength - 2ull, 16);

			buff[0] = '0';

			buff[1] = 'x';

			*messageStr += buff;
		}
		else
		{
			//-9223372036854775808\0 21
			const size_t decMaxLength = 21ull;

			char buff[decMaxLength] = {};

			_i64toa_s(arg, buff, decMaxLength, 10);

			*messageStr += buff;
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const uint32_t& arg)
	{
		setDisplayColor(LogColor::numericColor);

		if (integerMode == IntegerMode::HEX)
		{
			//0xFFFFFFFF\0 11
			const size_t hexMaxLength = 11ull;

			char buff[hexMaxLength] = {};

			_ultoa_s(arg, buff + 2, hexMaxLength - 2ull, 16);

			buff[0] = '0';

			buff[1] = 'x';

			*messageStr += buff;
		}
		else
		{
			//4294967295\0 11
			const size_t decMaxLength = 11ull;

			char buff[decMaxLength] = {};

			_ultoa_s(arg, buff, decMaxLength, 10);

			*messageStr += buff;
		}

		*messageStr += " ";
	}

	void LogContext::packArgument(const uint64_t& arg)
	{
		setDisplayColor(LogColor::numericColor);

		if (integerMode == IntegerMode::HEX)
		{
			//0xFFFFFFFFFFFFFFFF\0 19
			const size_t hexMaxLength = 19ull;

			char buff[hexMaxLength] = {};

			_ui64toa_s(arg, buff + 2, hexMaxLength - 2ull, 16);

			buff[0] = '0';

			buff[1] = 'x';

			*messageStr += buff;
		}
		else
		{
			//18446744073709551615 21
			const size_t decMaxLength = 21ull;

			char buff[decMaxLength] = {};

			_ui64toa_s(arg, buff, decMaxLength, 10);

			*messageStr += buff;
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
