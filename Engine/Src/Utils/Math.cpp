#include<Gear/Utils/Math.h>

namespace Gear::Utils::Math
{
	float lerp(const float x, const float y, const float s)
	{
		return x * (1.f - s) + y * s;
	}

	float clamp(const float value, const float min, const float max)
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

	float saturate(const float value)
	{
		return clamp(value, 0.f, 1.f);
	}

	float gauss(const float sigma, const float x)
	{
		return 1.f / (sigma * 2.506628274631000502415765284811f) * exp(-0.5f * (x / sigma) * (x / sigma));
	}

	uint32_t log2(const uint32_t x)
	{
		unsigned long y;

		_BitScanReverse(&y, x);

		return y;
	}
}