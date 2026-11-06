#pragma once

#ifndef _GEAR_UTILS_LOGGER_H_
#define _GEAR_UTILS_LOGGER_H_

#include "LogContext.h"

namespace Gear::Utils::Logger
{
	void submitLogMessage(const LogMessage& msg);
}

#endif // !_GEAR_UTILS_LOGGER_H_
