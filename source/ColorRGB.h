#pragma once
#include "MathHelpers.h"

namespace dae
{
	struct ColorRGBA
	{
		float r{};
		float g{};
		float b{};
		float a{ 1.f };

		void MaxToOne()
		{
			const float maxValue = std::max(r, std::max(g, b));
			if (maxValue > 1.f)
				*this /= maxValue;

			a = std::min(a, 1.f);
		}

		static ColorRGBA Lerp(const ColorRGBA& c1, const ColorRGBA& c2, float factor)
		{
			return { Lerpf(c1.r, c2.r, factor), Lerpf(c1.g, c2.g, factor), Lerpf(c1.b, c2.b, factor), Lerpf(c1.a, c2.b, factor) };
		}

#pragma region ColorRGBA (Member) Operators
		const ColorRGBA& operator+=(const ColorRGBA& c)
		{
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;

			return *this;
		}

		ColorRGBA operator+(const ColorRGBA& c) const
		{
			return { r + c.r, g + c.g, b + c.b, a + c.a };
		}

		const ColorRGBA& operator-=(const ColorRGBA& c)
		{
			r -= c.r;
			g -= c.g;
			b -= c.b;
			a -= c.a;

			return *this;
		}

		ColorRGBA operator-(const ColorRGBA& c) const
		{
			return { r - c.r, g - c.g, b - c.b, a - c.a };
		}

		const ColorRGBA& operator*=(const ColorRGBA& c)
		{
			r *= c.r;
			g *= c.g;
			b *= c.b;
			a *= c.a;

			return *this;
		}

		ColorRGBA operator*(const ColorRGBA& c) const
		{
			return { r * c.r, g * c.g, b * c.b, a * c.a };
		}

		const ColorRGBA& operator/=(const ColorRGBA& c)
		{
			r /= c.r;
			g /= c.g;
			b /= c.b;
			a /= c.a;

			return *this;
		}

		const ColorRGBA& operator*=(float s)
		{
			r *= s;
			g *= s;
			b *= s;
			a *= s;

			return *this;
		}

		ColorRGBA operator*(float s) const
		{
			return { r * s, g * s, b * s, a * s };
		}

		const ColorRGBA& operator/=(float s)
		{
			r /= s;
			g /= s;
			b /= s;
			a /= s;

			return *this;
		}

		ColorRGBA operator/(float s) const
		{
			return { r / s, g / s, b / s, a / s };
		}
#pragma endregion
	};

	//ColorRGBA (Global) Operators
	inline ColorRGBA operator*(float s, const ColorRGBA& c)
	{
		return c * s;
	}

	namespace colors
	{
		static ColorRGBA Red{ 1,0,0 };
		static ColorRGBA Blue{ 0,0,1 };
		static ColorRGBA Green{ 0,1,0 };
		static ColorRGBA Yellow{ 1,1,0 };
		static ColorRGBA Cyan{ 0,1,1 };
		static ColorRGBA Magenta{ 1,0,1 };
		static ColorRGBA White{ 1,1,1 };
		static ColorRGBA Black{ 0,0,0 };
		static ColorRGBA Gray{ 0.5f,0.5f,0.5f };
	}
}