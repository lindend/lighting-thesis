#pragma once
#include "Export.h"
#include "CrazeMath.h"

namespace Craze
{
	CRAZEMATH_EXP float Sqrt(float num);
	CRAZEMATH_EXP float RSqrt(float num);

	class CRAZEMATH_EXP Vector2
	{
	public:
			
		Vector2() {}
		inline Vector2(float _x, float _y) : x(_x), y(_y) {}

		void Set(float _x, float _y) { x = _x; y = _y; }

		inline float GetLen() const { return Craze::Sqrt(x * x + y * y); }
		inline float GetRLen() const { return Craze::RSqrt(x * x + y * y); }
		inline float GetSquareLen() const { return x * x + y * y; }

		inline void Normalize() { float rlen = GetRLen(); x *= rlen; y *= rlen; }

		inline const Vector2 operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
		inline const Vector2 operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
		inline const Vector2 operator*(float f) const { return Vector2(x * f, y * f); }
		inline const Vector2 operator-(float f) const { return Vector2(x - f, y - f); }

		inline const Vector2& operator+= (const Vector2& v) { x += v.x; y += v.y; return *this; }
		inline const Vector2& operator-= (const Vector2& v) { x -= v.x; y -= v.y; return *this; };

		inline bool operator==(const Vector2& v) { return Abs(x - v.x) < FDELTA && Abs(y - v.y) < FDELTA; }

		float x;
		float y;
	};

	inline const Vector2 operator*(const float& f, const Vector2& v) { return Vector2(v.x * f, v.y * f); }
	
}
