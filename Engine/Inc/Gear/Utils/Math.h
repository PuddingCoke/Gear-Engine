#pragma once

#ifndef _GEAR_UTILS_MATH_H_
#define _GEAR_UTILS_MATH_H_

#include<cmath>

#include<DirectXMath.h>

#include<DirectXPackedVector.h>

#include<type_traits>

namespace Gear::Utils::Math
{
	constexpr float halfPi = DirectX::XM_PIDIV2;

	constexpr float pi = DirectX::XM_PI;

	constexpr float twoPi = DirectX::XM_2PI;

	constexpr float radToDeg = 180.f / Math::pi;

	constexpr float degToRad = 1.f / radToDeg;

	constexpr float lerp(const float x, const float y, const float s)
	{
		return x * (1.f - s) + y * s;
	}

	constexpr float clamp(const float value, const float min, const float max)
	{
		if (value < min)
		{
			return min;
		}
		else if (value > max)
		{
			return max;
		}

		return value;
	}

	constexpr float saturate(const float value)
	{
		return clamp(value, 0.f, 1.f);
	}

	template<typename T>
	constexpr T ceil(const T a, const T b)
	{
		static_assert(std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>, "Math::ceil only support uint32_t and uint64_t");

		return (a + b - static_cast<T>(1)) / b;
	}

	//a是否拥有b？
	template<typename T>
	constexpr bool bitFlagSubset(const T a, const T b)
	{
		static_assert(std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>, "Math::bitFlagSubset only support uint32_t and uint64_t");

		return b && ((a & b) == b);
	}

	float gauss(const float sigma, const float x);

	float pow(const float x, const float y);

	uint32_t log2(const uint32_t x);
}

#endif // !_GEAR_UTILS_MATH_H_