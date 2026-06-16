#pragma once

#ifndef _GEAR_UTILS_LOGGER_H_
#define _GEAR_UTILS_LOGGER_H_

#include"Logger/LogContext.h"

#include"String.h"

using IntegerMode = Gear::Utils::Logger::LogContext::IntegerMode;

using FloatPrecision = Gear::Utils::Logger::LogContext::FloatPrecision;

using LogColor = Gear::Utils::Logger::LogColor;

/// <summary>
/// 一个可以输出不同颜色的日志记录器
/// int32_t、uint32_t、float_t这类数值类型会有特殊的颜色
/// 因为LogContext目前的实现，窄字符串是不被支持的，在编译时会报错
/// 此外最好不要在循环中使用日志记录器
/// 
/// 以下是支持的数据类型
/// 整形：int32_t、int64_t、uint32_t、uint64_t
/// 浮点：float_t、double_t
/// 字符串：字面宽字符串、const wchar_t*、std::wstring
/// 
/// 你可以使用IntegerType来指定整数的输出进制，下方为示例代码
/// LOGUSER(L"32位无符号整数测试", IntegerMode::HEX, 13689u, UINT_MAX, IntegerMode::DEC, 13689u, UINT_MAX);
/// 
/// 你可以使用FloatPrecision来调整小数点后位数，下方为示例代码
/// such as LOGUSER(L"浮点测试", FloatPrecision(4), 125.6f, FloatPrecision(2), 125.7);
/// 
/// 在LogColor这个类中有很多可用的颜色
/// </summary>
namespace Gear::Utils::Logger
{
	void submitLogMessage(const LogMessage& msg);
}

#define TOSTRING(x) #x

#define TOWSTRING(x) L#x

#define LOGSUCCESS(...) Gear::Utils::Logger::submitLogMessage(Gear::Utils::Logger::LogContext::createLogMessage(__FUNCTIONW__,Gear::Utils::Logger::LogType::LOG_SUCCESS,__VA_ARGS__))

#define LOGENGINE(...) Gear::Utils::Logger::submitLogMessage(Gear::Utils::Logger::LogContext::createLogMessage(__FUNCTIONW__,Gear::Utils::Logger::LogType::LOG_ENGINE,__VA_ARGS__))

#define LOGUSER(...) Gear::Utils::Logger::submitLogMessage(Gear::Utils::Logger::LogContext::createLogMessage(__FUNCTIONW__,Gear::Utils::Logger::LogType::LOG_USER,__VA_ARGS__))

#define LOGERROR(...) do { \
const Gear::Utils::Logger::LogMessage _logMessage_ = Gear::Utils::Logger::LogContext::createLogMessage(__FUNCTIONW__,Gear::Utils::Logger::LogType::LOG_ERROR,__FILEW__,L"LINE",static_cast<int32_t>(__LINE__),__VA_ARGS__); \
const std::string _errorStr_ = Gear::Utils::String::wstringToStr(_logMessage_.slot.str); \
Gear::Utils::Logger::submitLogMessage(_logMessage_); \
throw std::runtime_error(_errorStr_); \
} while(0)\

#endif // !_GEAR_UTILS_LOGGER_H_