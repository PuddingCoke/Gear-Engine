#pragma once

#ifndef _GEAR_UTILS_LOGGER_LOGCONTEXT_H_
#define _GEAR_UTILS_LOGGER_LOGCONTEXT_H_

#include"LogColor.h"

#include<Gear/Utils/Memory.h>

#include<thread>

#include<condition_variable>

#include<array>

namespace Gear::Utils::Logger
{
	enum class LogType
	{
		LOG_SUCCESS,
		LOG_ERROR,
		LOG_ENGINE,
		LOG_USER
	};

	struct LogMessage
	{
		const std::wstring& str;

		const LogType type;

		std::mutex& inUseMutex;

		std::condition_variable& inUseCV;

		uint64_t& readIndex;
	};

	class LogContext
	{
	public:

		enum class IntegerMode
		{
			DEC, HEX
		};

		struct FloatPrecision
		{
			FloatPrecision(const int32_t precision = 5);

			int32_t precision;
		};

		LogContext(const LogContext&) = delete;

		void operator=(const LogContext&) = delete;

		LogContext();

		~LogContext();

		template<typename... Args>
		static LogMessage createLogMessage(const wchar_t* const functionName, const LogType& type, const Args&... args);

	private:

		template<typename... Args>
		LogMessage getLogMessage(const wchar_t* const functionName, const LogType& type, const Args&... args);

		template<typename T>
		struct isNativeString :std::false_type {};

		template<size_t N>
		struct isNativeString<const char[N]> : std::true_type {};

		template<size_t N>
		struct isNativeString<char[N]> : std::true_type {};

		template<>
		struct isNativeString<const char*> : std::true_type {};

		template<>
		struct isNativeString<char*> : std::true_type {};

		template<typename First, typename... Rest>
		void packRestArgument(const First& first, const Rest&... rest);

		void packRestArgument();

		//匹配模板函数是禁止的，在这种情况下会抛出编译错误
		template<typename Arg>
		void packArgument(const Arg& arg);

		//宽字符串
		void packArgument(const std::wstring& arg);

		//原生宽字符串
		void packArgument(const wchar_t* arg);

		//有符号32位整数
		void packArgument(const int32_t& arg);

		//有符号64位整数
		void packArgument(const int64_t& arg);

		//无符号32位整数
		void packArgument(const uint32_t& arg);

		//无符号64位整数
		void packArgument(const uint64_t& arg);

		template<typename Arg>
		void packFloatPoint(const Arg& arg);

		//浮点数
		void packArgument(const float_t& arg);

		//双精度浮点数
		void packArgument(const double_t& arg);

		//改变整数模式
		void packArgument(const IntegerMode& mode);

		//改变浮点精度
		void packArgument(const FloatPrecision& precision);

		//改变正文颜色
		void packArgument(const LogColor& arg);

		//改变显示颜色
		void setDisplayColor(const LogColor& color);

		void resetState();

		IntegerMode integerMode;

		FloatPrecision floatPrecision;

		LogColor textColor;

		LogColor displayColor;

		static constexpr size_t slotNum = 128ull;

		std::array<std::wstring, slotNum> slots;

		uint64_t writeIndex;

		uint64_t readIndex;

		std::wstring* messageStr;

		std::mutex inUseMutex;

		std::condition_variable inUseCV;

	};

	template<typename ...Args>
	inline LogMessage LogContext::createLogMessage(const wchar_t* const functionName, const LogType& type, const Args & ...args)
	{
		thread_local UniquePtr<LogContext> context = makeUnique<LogContext>();

		context->resetState();

		return context->getLogMessage(functionName, type, args...);
	}

	template<typename ...Args>
	inline LogMessage LogContext::getLogMessage(const wchar_t* const functionName, const LogType& type, const Args & ...args)
	{
		writeIndex++;

		if (!(writeIndex - readIndex < slotNum))
		{
			std::unique_lock<std::mutex> inUseLock(inUseMutex);

			inUseCV.wait(inUseLock, [this]() { return writeIndex - readIndex < slotNum; });
		}

		const uint64_t modWriteIndex = writeIndex % slotNum;

		messageStr = &slots[modWriteIndex];

		const LogMessage message = { slots[modWriteIndex],type,inUseMutex,inUseCV,readIndex };

		messageStr->clear();

		{
			const time_t currentTime = time(nullptr);

			tm localTime = {};

			localtime_s(&localTime, &currentTime);

			//headerStrLen = 5+2+8+1+5+2+1+10+1+5+2+length(functionName)+1+1
			//			   = 44+length(functionName)
			const size_t headerStrLen = 256ull;

			wchar_t headerStr[headerStrLen] = {};

			const std::thread::id id = std::this_thread::get_id();

			const uint32_t threadId = *(uint32_t*)&id;

			swprintf_s(headerStr, headerStrLen, L"%s[%d:%d:%d] %s{T%u} %s(%s) ", LogColor::timeStampColor.code, localTime.tm_hour, localTime.tm_min, localTime.tm_sec,
				LogColor::threadIdColor.code, threadId, LogColor::functionNameColor.code, functionName);

			*messageStr += headerStr;
		}

		switch (type)
		{
		case LogType::LOG_SUCCESS:

			packArgument(LogColor::successColor);

			packArgument(L"<SUCCESS>");

			break;
		case LogType::LOG_ERROR:

			packArgument(LogColor::errorColor);

			packArgument(L"<ERROR>");

			break;
		case LogType::LOG_ENGINE:

			packArgument(LogColor::engineColor);

			packArgument(L"<ENGINE>");

			break;
		case LogType::LOG_USER:

			packArgument(LogColor::userColor);

			packArgument(L"<USER>");

			break;
		}

		packArgument(LogColor::defaultColor);

		packRestArgument(args...);

		return message;
	}

	template<typename First, typename ...Rest>
	inline void LogContext::packRestArgument(const First& first, const Rest& ...rest)
	{
		static_assert(!std::is_same<std::string, First>::value, "error input type is std::string");

		static_assert(!isNativeString<First>::value, "error input type is native string");

		packArgument(first);

		packRestArgument(rest...);
	}

	template<typename Arg>
	inline void LogContext::packArgument(const Arg& arg)
	{
		static_assert(0, "not supported type");

		//用于测试
		/*setDisplayColor(textColor);

		const std::string ty = typeid(Arg).name();

		packArgument(std::wstring(ty.cbegin(), ty.cend()));*/
	}

	template<typename Arg>
	inline void LogContext::packFloatPoint(const Arg& arg)
	{
		setDisplayColor(LogColor::numericColor);

		wchar_t buff[_CVTBUFSIZE] = {};

		swprintf_s(buff, _CVTBUFSIZE, L"%.*f ", floatPrecision.precision, arg);

		*messageStr += buff;
	}
}

#endif // !_GEAR_UTILS_LOGGER_LOGCONTEXT_H_
