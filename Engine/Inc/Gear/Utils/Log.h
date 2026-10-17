#pragma once

#ifndef _GEAR_UTILS_LOGGER_LOG_H_
#define _GEAR_UTILS_LOGGER_LOG_H_

/// <summary>
/// 一个可以输出不同颜色的日志记录器
/// int32_t、uint32_t、float_t这类数值类型会有特殊的颜色
/// 最好不要在循环中使用日志记录器
/// 
/// 以下是支持的数据类型
/// 整形：int32_t、int64_t、uint32_t、uint64_t
/// 浮点：float_t、double_t
/// 字符串：字面窄字符串、const char*、std::string、字面宽字符串、const wchar_t*、std::wstring、字面u8字符串、const char8_t*、std::u8string
/// 布尔类型：会被转换为亮绿色的TRUE和亮红色的FALSE
/// 
/// 你可以使用LogIntegerMode来指定整数的输出进制，下方为示例代码
/// LOGUSER() << "32位无符号整数测试" << LogIntegerMode::HEX << 13689u << UINT_MAX << LogIntegerMode::DEC << 13689u << UINT_MAX;
/// 
/// 你可以使用LogFloatPrecision来调整小数点后位数，下方为示例代码
/// LOGUSER() << "32位浮点测试" << LogFloatPrecision(4) << 125.6f << FLT_MAX << LogFloatPrecision(2) << 125.6f << FLT_MAX;
/// 
/// LogColor这个结构体中有很多可用的颜色
/// </summary>

#include"Logger/LogContext.h"

namespace Gear::Utils::Logger
{
	class Log
	{
	public:

		Log(const std::string_view& functionName, const LogType& type);

		~Log();

		template<typename T>
		const Log& operator<<(const T& arg) const;

		void finishAndThrowMessage() const;

	private:

		bool needThrow() const;

		void finishAndPostMessage() const;

		LogContext& context;

	};

	template<typename T>
	inline const Log& Log::operator<<(const T& arg) const
	{
		context.packArgument(arg);

		return *this;
	}

	[[noreturn]] void ThrowLog(const Log& log);

}

constexpr std::string_view getShortFuncName(const char* const funcName)
{
	std::string_view sv(funcName);

	const auto internalPos = sv.rfind("::Internal::");

	if (internalPos != std::string_view::npos)
	{
		if (internalPos == 0ull)
		{
			return sv;
		}

		const auto prevPos = sv.rfind("::", internalPos - 1ull);

		//ASD::ASD::Internal::ASD -> ASD::Internal::ASD
		if (prevPos != std::string_view::npos)
		{
			return sv.substr(prevPos + 2ull);
		}
		//ASD::Internal::ASD -> ASD::Internal::ASD
		else
		{
			return sv;
		}
	}
	else
	{
		const auto prevPos = sv.rfind("::");

		//ASDFGH -> ASDFGH
		if (prevPos == std::string_view::npos)
		{
			return sv;
		}
		else
		{
			//::ASDFGH -> ::ASDFGH
			if (prevPos == 0ull)
			{
				return sv;
			}

			const auto prevPrevPos = sv.rfind("::", prevPos - 1ull);

			//ASDFG::ASDFG::ASDFG -> ASDFG::ASDFG
			if (prevPrevPos != std::string_view::npos)
			{
				return sv.substr(prevPrevPos + 2ull);
			}
			//ASDFG::ASDFG -> ASDFG::ASDFG
			else
			{
				return sv;
			}
		}
	}
}

using Gear::Utils::Logger::LogIntegerMode;

using Gear::Utils::Logger::LogFloatPrecision;

using Gear::Utils::Logger::LogColor;

using Gear::Utils::Logger::LogNewLine;

#define TOSTRING(x) #x

#define TOWSTRING(x) L#x

#define TOSHORTFUNCNAME(x) getShortFuncName(x)

#define LOGSUCCESS() Gear::Utils::Logger::Log(TOSHORTFUNCNAME(__FUNCTION__),Gear::Utils::Logger::LogType::LOG_SUCCESS)

#define LOGENGINE() Gear::Utils::Logger::Log(TOSHORTFUNCNAME(__FUNCTION__),Gear::Utils::Logger::LogType::LOG_ENGINE)

#define LOGUSER() Gear::Utils::Logger::Log(TOSHORTFUNCNAME(__FUNCTION__),Gear::Utils::Logger::LogType::LOG_USER)

#define LOGERROR() Gear::Utils::Logger::Log(TOSHORTFUNCNAME(__FUNCTION__),Gear::Utils::Logger::LogType::LOG_ERROR) << __FILE__ << "LINE" << static_cast<int32_t>(__LINE__)

#define THROWLOG(_log_) Gear::Utils::Logger::ThrowLog(_log_)

#define COLORIZE(_content_ , _color_) _color_ << _content_ << LogColor::defaultColor

#define COLORIZESTRUCT(_struct_) COLORIZE(TOSTRING(_struct_), LogColor::brightMagenta)

#define COLORIZEVAR(_var_) COLORIZE(TOSTRING(_var_), LogColor::brightYellow)

#define COLORIZEENUM(_enum_) COLORIZE(TOSTRING(_enum_), LogColor::blue)

#define COLORIZEPATH(_path_) COLORIZE(_path_, LogColor::brightBlue)

#endif // !_GEAR_UTILS_LOGGER_LOG_H_
