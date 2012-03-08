#pragma once

#include "CrazeMath.h"

#include "Vector3.h"

namespace Craze
{
	inline float LengthSquared(const Vector3& v)
	{
		__m128 sq = _mm_mul_ps(v.vec, v.vec);
		return sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
	}
	inline float Length(const Vector3& v)
	{
		return Sqrt(LengthSquared(v));
	}
	inline float RLength(const Vector3& v)
	{
		return RSqrt(LengthSquared(v));
	}

	inline Vector3 Normalize(const Vector3& v)
	{
		return v * RLength(v);
	}

	inline Vector3 Min(const Vector3& v0, const Vector3& v1)
	{
		return Vector3(_mm_min_ps(v0.vec, v1.vec));
	}

	inline Vector3 Max(const Vector3& v0, const Vector3& v1)
	{
		return Vector3(_mm_max_ps(v0.vec, v1.vec));
	}

	inline Vector3 Cross(const Vector3& v0, const Vector3& v1)
	{
		#define YZXSHUF(vect) _mm_shuffle_ps(vect, vect, _MM_SHUFFLE(3, 0, 2, 1))
		#define ZXYSHUF(vect) _mm_shuffle_ps(vect, vect, _MM_SHUFFLE(3, 1, 0, 2))

		Vector3 v(_mm_sub_ps(_mm_mul_ps(YZXSHUF(v0.vec), ZXYSHUF(v1.vec)), _mm_mul_ps(ZXYSHUF(v0.vec), YZXSHUF(v1.vec))));
		return v;

		#undef YZXSHUF
		#undef ZXYSHUF
	}

	inline float Dot(const Vector3& v0, const Vector3& v1)
	{
		__m128 p = _mm_mul_ps(v0.vec, v1.vec);
		return (p.m128_f32[0] + p.m128_f32[1] + p.m128_f32[2]);
	}
}