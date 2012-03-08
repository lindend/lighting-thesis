#pragma once
#include "Export.h"

#include "Memory/MemoryManager.h"

#include "mmintrin.h"
#include "xmmintrin.h"

#pragma warning(push)
//Non standard extention, nameless struct/union
#pragma warning(disable: 4201)

namespace Craze
{
	class Vector3;
	class Vector4;
	struct Vec4;

	struct CRAZEMATH_EXP Vec3
	{
		float x;
		float y;
		float z;

		operator Vector3() const;
		operator Vector4() const;
		operator Vec4() const;
	};

	CRAZEMATH_EXP Vec3 MkV3(float _x, float _y, float _z);


	__declspec(align(16)) class CRAZEMATH_EXP Vector3
	{
		CRAZE_POOL_ALLOC_ALIGN(Vector3, 16);
	public:
		Vector3() {}
		Vector3(__m128 v) { vec = v; }
		Vector3(float v) { vec = _mm_setr_ps(v, v, v, 0.0f); }
		Vector3(float _x, float _y, float _z);
			
		void Set(float _x, float _y, float _z);
		float GetX() const { return vec.m128_f32[0]; }
		float GetY() const { return vec.m128_f32[1]; }
		float GetZ() const { return vec.m128_f32[2]; }

		float GetLen() const;
		float GetRLen() const;

		float GetSquaredLen() const;

		const Vector3 operator-() const { return _mm_sub_ps(_mm_setzero_ps(), vec); }

		void Normalize();
		Vector3 Normalized() const;
		Vector3 Cross(const Vector3& v) const;
		static Vector3 Normalize(const Vector3& v);

		float Dot(const Vector3& v) const;

		static Vector3 Max(const Vector3& v1, const Vector3& v2);
		static Vector3 Min(const Vector3& v1, const Vector3& v2);

		operator Vec3() const { return v; }
		Vec3* operator->() { return &v; }
		const Vec3* operator->() const { return &v; }

		float operator[](unsigned int idx) const { return vec.m128_f32[idx]; }

		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};

			//Kept so I don't have to rewrite stuff, even though it is pretty awful
			struct
			{
				Vec3 v;
				float _pad;
			};

			__m128 vec;
		};

		static const Vector3 FORWARD;
		static const Vector3 BACKWARD;
		static const Vector3 UP;
		static const Vector3 DOWN;
		static const Vector3 RIGHT;
		static const Vector3 LEFT;

		static const Vector3 ZERO;
		static const Vector3 ONE;
	};

	inline const Vector3 operator-(const Vector3& v0, const Vector3& v1) { return _mm_sub_ps(v0.vec, v1.vec); }
	inline const Vector3 operator+(const Vector3& v0, const Vector3& v1) { return _mm_add_ps(v0.vec, v1.vec); }
	inline const Vector3 operator*(const Vector3& v0, const Vector3& v1) { return _mm_mul_ps(v0.vec, v1.vec); }
	inline const Vector3 operator/(const Vector3& v0, const Vector3& v1) { return _mm_div_ps(v0.vec, v1.vec); }

	inline void operator+=(Vector3& v0, const Vector3& v1) { v0 = v0 + v1; }
	inline void operator-=(Vector3& v0, const Vector3& v1) { v0 = v0 - v1; }

	inline const Vector3 operator*(const Vector3& v, const float f) { return _mm_mul_ps(v.vec, _mm_set_ps1(f)); }
	inline const Vector3 operator/(const Vector3& v, const float f) { return _mm_mul_ps(v.vec, _mm_set_ps1(f)); }

	inline const Vector3 operator*(const float f, const Vector3& v) { return _mm_mul_ps(v.vec, _mm_set_ps1(f)); }
	inline const Vector3 operator/(const float f, const Vector3& v) { return _mm_mul_ps(v.vec, _mm_set_ps1(f)); }

	inline void operator*=(Vector3& v, float f) { v = v * f; }
	inline void operator/=(Vector3& v, float f) { v = v / f; }
	
}

#pragma warning(pop)