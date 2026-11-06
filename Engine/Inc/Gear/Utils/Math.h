#pragma once

#ifndef _GEAR_UTILS_MATH_H_
#define _GEAR_UTILS_MATH_H_

#include <cmath>

#include <algorithm>

#include <DirectXMath.h>

#include <DirectXPackedVector.h>

#include <type_traits>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace Gear::Utils::Math
{
	constexpr float halfPi = DirectX::XM_PIDIV2;

	constexpr float pi = DirectX::XM_PI;

	constexpr float twoPi = DirectX::XM_2PI;

	constexpr float radToDeg = 180.f / Math::pi;

	constexpr float degToRad = 1.f / radToDeg;

	template<typename T>
		requires std::is_floating_point_v<T>
	constexpr T lerp(const T x, const T y, const T s)
	{
		return x * (static_cast<T>(1) - s) + y * s;
	}

	template<typename T>
		requires std::is_floating_point_v<T> || std::is_integral_v<T>
	constexpr T min(const T a, const T b)
	{
		return std::min(a, b);
	}

	template<typename T>
		requires std::is_floating_point_v<T> || std::is_integral_v<T>
	constexpr T max(const T a, const T b)
	{
		return std::max(a, b);
	}

	template<typename T>
		requires std::is_floating_point_v<T> || std::is_integral_v<T>
	constexpr T clamp(const T value, const T min, const T max)
	{
		return std::clamp(value, min, max);
	}

	template<typename T>
		requires std::is_floating_point_v<T>
	constexpr T saturate(const T value)
	{
		return clamp(value, static_cast<T>(0), static_cast<T>(1));
	}

	template<typename T>
		requires std::is_floating_point_v<T>
	T floor(const T value)
	{
		return std::floor(value);
	}

	template<typename T>
		requires std::is_floating_point_v<T>
	T ceil(const T value)
	{
		return std::ceil(value);
	}

	template<typename T>
		requires std::is_floating_point_v<T>
	T divFloor(const T a, const T b)
	{
		return floor(a / b);
	}

	template<typename T>
		requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> ||
	             std::is_same_v<T, uint64_t>
	constexpr T divFloor(const T a, const T b)
	{
		return a / b;
	}

	template<typename T>
		requires std::is_floating_point_v<T>
	T divCeil(const T a, const T b)
	{
		return ceil(a / b);
	}

	template<typename T>
		requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> ||
	             std::is_same_v<T, uint64_t>
	constexpr T divCeil(const T a, const T b)
	{
		return (a + b - static_cast<T>(1)) / b;
	}

	template<typename T>
		requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> ||
	             std::is_same_v<T, uint64_t>
	constexpr T align(const T a, const T b)
	{
		return divCeil(a, b) * b;
	}

	//a是否拥有b？
	template<typename T>
		requires std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>
	constexpr bool bitFlagSubset(const T a, const T b)
	{
		return b && ((a & b) == b);
	}

	template<typename T>
		requires std::is_floating_point_v<T> || std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
	             std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>
	T sign(const T x)
	{
		return static_cast<T>(1) - static_cast<T>(2) * static_cast<T>(std::signbit(x));
	}

	template<typename T>
		requires std::is_floating_point_v<T>
	T pow(const T x, const T y)
	{
		return std::pow(x, y);
	}

	template<typename T>
		requires std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>
	uint32_t log2(const T x)
	{
		unsigned long y;

		bool nonZero;

		if constexpr (std::is_same_v<T, uint32_t>)
		{
			nonZero = _BitScanReverse(&y, x);
		}
		else
		{
			nonZero = _BitScanReverse64(&y, x);
		}

		return static_cast<uint32_t>(nonZero) & static_cast<uint32_t>(y);
	}

	template<typename T>
		requires std::is_floating_point_v<T>
	T gauss(const T sigma, const T x)
	{
		if constexpr (std::is_same_v<T, float>)
		{
			return 1.0f / (sigma * 2.506628274631000502415765284811f) *
			       std::exp(-0.5f * (x / sigma) * (x / sigma));
		}
		else
		{
			return 1.0 / (sigma * 2.506628274631000502415765284811) *
			       std::exp(-0.5 * (x / sigma) * (x / sigma));
		}
	}
}

#endif // !_GEAR_UTILS_MATH_H_
