#pragma once

#ifndef _GEAR_UTILS_COLOR_H_
#define _GEAR_UTILS_COLOR_H_

#include<cstdint>

#include<DirectXColors.h>

namespace Gear::Utils
{
	struct Color
	{

		float r;

		float g;

		float b;

		float a;

		uint32_t toUint() const;

		bool operator==(const Color& color) const;

		bool operator!=(const Color& color) const;

		operator const float* () const;

		static Color random();

		static Color hsvToRgb(const Color& c);

	};
}

#endif // !_GEAR_UTILS_COLOR_H_