#include<Gear/Utils/Math.h>

namespace Gear::Utils::Math
{
	float gauss(const float sigma, const float x)
	{
		return 1.f / (sigma * 2.506628274631000502415765284811f) * exp(-0.5f * (x / sigma) * (x / sigma));
	}

	float pow(const float x, const float y)
	{
		return powf(x, y);
	}

	uint32_t log2(const uint32_t x)
	{
		unsigned long y;

		_BitScanReverse(&y, x);

		return y;
	}
}