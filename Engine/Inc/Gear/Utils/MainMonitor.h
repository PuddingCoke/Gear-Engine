#pragma once

#ifndef _GEAR_UTILS_MAINMONITOR_H_
#define _GEAR_UTILS_MAINMONITOR_H_

#include<cstdint>

namespace Gear::Utils::MainMonitor
{
	//主监视器宽
	uint32_t getWidth();

	//主监视器高
	uint32_t getHeight();

	//主监视器刷新率
	uint32_t getRefreshRate();

	//主监视器放大比率
	float getScale();
}

#endif // !_GEAR_UTILS_MAINMONITOR_H_