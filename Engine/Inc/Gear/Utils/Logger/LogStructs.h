#pragma once

#ifndef _GEAR_UTILS_LOGGER_LOGSTRUCTS_H_
#define _GEAR_UTILS_LOGGER_LOGSTRUCTS_H_

#include <cstdint>

namespace Gear::Utils::Logger
{
	enum class LogType
	{
		LOG_SUCCESS,
		LOG_ERROR,
		LOG_ENGINE,
		LOG_USER
	};

	enum class LogIntegerMode
	{
		DEC,
		HEX
	};

	struct LogColor
	{
		const char* code;

		bool operator!=(const LogColor& color) const;

		static const LogColor black;

		static const LogColor red;

		static const LogColor green;

		static const LogColor yellow;

		static const LogColor blue;

		static const LogColor magenta;

		static const LogColor cyan;

		static const LogColor white;

		static const LogColor brightBlack;

		static const LogColor brightRed;

		static const LogColor brightGreen;

		static const LogColor brightYellow;

		static const LogColor brightBlue;

		static const LogColor brightMagenta;

		static const LogColor brightCyan;

		static const LogColor brightWhite;

		//默认正文颜色
		static const LogColor defaultColor;

		static const LogColor numericColor;

		static const LogColor timeStampColor;

		static const LogColor threadIdColor;

		static const LogColor functionNameColor;

		static const LogColor successColor;

		static const LogColor errorColor;

		static const LogColor engineColor;

		static const LogColor userColor;
	};

	struct LogFloatPrecision
	{
		LogFloatPrecision(const int32_t precision = 5);

		int32_t precision;
	};

	struct LogNewLine
	{
	};
}

#endif // !_GEAR_UTILS_LOGGER_LOGSTRUCTS_H_
