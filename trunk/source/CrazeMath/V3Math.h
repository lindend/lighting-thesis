#pragma once

#include "CrazeMath.h"

#include "Vector3.h"

namespace Craze
{
	inline const float LengthSquared(const Vector3& v)
	{
		__m128 sq = _mm_mul_ps(v.vec, v.vec);
		return sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
	}
	inline const float Length(const Vector3& v)
	{
        const float sqLen = LengthSquared(v);
		const float len = Sqrt(sqLen);
        return len;
	}
	inline const float RLength(const Vector3& v)
	{
		return RSqrt(LengthSquared(v));
	}

	inline const Vector3 Normalize(const Vector3& v)
	{
		return v * RLength(v);
	}

	inline const Vector3 Min(const Vector3& v0, const Vector3& v1)
	{
		return Vector3(_mm_min_ps(v0.vec, v1.vec));
	}

	inline const Vector3 Max(const Vector3& v0, const Vector3& v1)
	{
		return Vector3(_mm_max_ps(v0.vec, v1.vec));
	}

	inline const Vector3 Cross(const Vector3& v0, const Vector3& v1)
	{
		#define YZXSHUF(vect) _mm_shuffle_ps(vect, vect, _MM_SHUFFLE(3, 0, 2, 1))
		#define ZXYSHUF(vect) _mm_shuffle_ps(vect, vect, _MM_SHUFFLE(3, 1, 0, 2))

		Vector3 v(_mm_sub_ps(_mm_mul_ps(YZXSHUF(v0.vec), ZXYSHUF(v1.vec)), _mm_mul_ps(ZXYSHUF(v0.vec), YZXSHUF(v1.vec))));
		return v;

		#undef YZXSHUF
		#undef ZXYSHUF
	}

	inline const float Dot(const Vector3& v0, const Vector3& v1)
	{
		__m128 p = _mm_mul_ps(v0.vec, v1.vec);
		return (p.m128_f32[0] + p.m128_f32[1] + p.m128_f32[2]);
	}

	inline const Vector3 Sign(const Vector3& v)
	{
		//Mask away the -1 or 1 if the number equals zero
							//Check if the number is not zero and give a mask that we can use
		return _mm_and_ps(	_mm_cmpeq_ps(v.vec, Vector3::ZERO.vec),
							//Add the sign bit that has been isolated to a 1
							_mm_or_ps(_mm_set1_ps(1.f),
									//Isolate the sign bit
									_mm_and_ps(v.vec, _mm_set1_ps(-0.f))));
	}

	inline const Vector3 Ceil(const Vector3& v)
	{
		return _mm_ceil_ps(v.vec);
	}
	inline const Vector3 Floor(const Vector3& v)
	{
		return _mm_floor_ps(v.vec);
	}
}