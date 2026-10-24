#pragma once

#ifndef _GEAR_UTILS_LOGGER_LOGCONTEXT_H_
#define _GEAR_UTILS_LOGGER_LOGCONTEXT_H_

#include"LogStructs.h"

#include<Gear/Utils/File.h>

#include<Gear/Utils/Memory.h>

#include<mutex>

#include<condition_variable>

#include<array>

#include<type_traits>

#include<string_view>

namespace Gear::Utils::Logger
{
	class LogContext;

	struct LogMessage
	{
		const std::string& str;

		LogContext* const context;

		const LogType type;
	};

	class LogContext
	{
	public:

		LogContext(const LogContext&) = delete;

		void operator=(const LogContext&) = delete;

		LogContext();

		~LogContext();

		static LogContext& get();

		void readIndexIncrement();

		//创建新的日志消息
		void createLogMessage(const std::string_view& functionName, const LogType& type);

		//宽字符串
		void packArgument(const std::wstring& arg);

		//原生宽字符串
		void packArgument(const wchar_t* arg);

		//窄字符串
		void packArgument(const std::string& arg);

		//原生窄字符串
		void packArgument(const char* arg);

		//UTF8字符串
		void packArgument(const std::u8string& arg);

		//原生UTF8字符串
		void packArgument(const char8_t* arg);

		//布尔类型
		void packArgument(const bool& arg);

		//有符号32位整数
		void packArgument(const int32_t& arg);

		//有符号64位整数
		void packArgument(const int64_t& arg);

		//无符号32位整数
		void packArgument(const uint32_t& arg);

		//无符号64位整数
		void packArgument(const uint64_t& arg);

		//单精度浮点数
		void packArgument(const float_t& arg);

		//双精度浮点数
		void packArgument(const double_t& arg);

		//文件路径
		void packArgument(const File::Path& arg);

		//改变整数模式
		void packArgument(const LogIntegerMode& arg);

		//改变浮点精度
		void packArgument(const LogFloatPrecision& arg);

		//改变正文颜色
		void packArgument(const LogColor& arg);

		//换行
		void packArgument(const LogNewLine&);

		LogType getLogType() const;

		LogMessage getLogMessage();

	private:

		template<typename Arg>
			requires std::is_floating_point_v<Arg>
		void packFloatPoint(const Arg& arg);

		//重置状态
		void resetState();

		//获取可复用的槽位
		void acquireReusableSlot();

		//添加头部信息
		void packHeader(const std::string_view& functionName, const LogType& type);

		//改变显示颜色
		void setDisplayColor(const LogColor& color);

		LogType logType;

		LogIntegerMode integerMode;

		LogFloatPrecision floatPrecision;

		LogColor textColor;

		LogColor displayColor;

		static constexpr size_t slotNum = 128ull;

		std::array<std::string, slotNum> slots;

		uint64_t writeIndex;

		uint64_t readIndex;

		std::string* messageStr;

		std::mutex readIndexMutex;

		std::condition_variable readIndexCV;

		static constexpr uint64_t convertBufferLength = 512ull;

		char convertBuffer[convertBufferLength];

	};

	template<typename Arg>
		requires std::is_floating_point_v<Arg>
	inline void LogContext::packFloatPoint(const Arg& arg)
	{
		setDisplayColor(LogColor::numericColor);

		sprintf_s(convertBuffer, convertBufferLength, "%.*f ", floatPrecision.precision, arg);

		*messageStr += convertBuffer;
	}
}

#endif // !_GEAR_UTILS_LOGGER_LOGCONTEXT_H_
