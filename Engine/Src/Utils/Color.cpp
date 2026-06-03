#include<Gear/Utils/Color.h>

#include<Gear/Utils/Random.h>

namespace Gear::Utils
{
	uint32_t Color::toUint() const
	{
		return (static_cast<uint32_t>(255 * a) << 24) | (static_cast<uint32_t>(255 * b) << 16) | (static_cast<uint32_t>(255 * g) << 8) | (static_cast<uint32_t>(255 * r));
	}

	Color Color::random()
	{
		return { Random::genFloat(),Random::genFloat() ,Random::genFloat(),1.f };
	}

	Color Color::hsvToRgb(const Color& c)
	{
		float r, g, b, f, p, q, t;

		const int32_t i = static_cast<int32_t>(floorf(c.r * 6.f));

		f = c.r * 6 - i;

		p = c.b * (1 - c.g);

		q = c.b * (1 - f * c.g);

		t = c.b * (1 - (1 - f) * c.g);

		switch (i % 6)
		{
		default:
		case 0: r = c.b, g = t, b = p; break;
		case 1: r = q, g = c.b, b = p; break;
		case 2: r = p, g = c.b, b = t; break;
		case 3: r = p, g = q, b = c.b; break;
		case 4: r = t, g = p, b = c.b; break;
		case 5: r = c.b, g = p, b = q; break;
		}

		return { r,g,b,1.f };
	}

	bool Color::operator==(const Color& color) const
	{
		return toUint() == color.toUint();
	}

	bool Color::operator!=(const Color& color) const
	{
		return toUint() != color.toUint();
	}

	Color::operator const float* () const
	{
		return reinterpret_cast<const float*>(this);
	}
}
