#pragma once

#ifndef _GEAR_UTILS_RANDOM_H_
#define _GEAR_UTILS_RANDOM_H_

#include<cstdint>

namespace Gear::Utils::Random
{
	//均值 0 标准差 1
	float genGauss();

	float genFloat();

	double genDouble();

	int32_t genInt();

	uint32_t genUint();
}

#endif // !_GEAR_UTILS_RANDOM_H_