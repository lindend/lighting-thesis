#pragma once

#include "Export.h"
#include "Vector3.h"

#pragma warning(push)
//Non standard extention, nameless struct/union
#pragma warning(disable: 4201)

namespace Craze
{
	class Vector4;

	struct CRAZEMATH_EXP Vec4
	{
        static Vec4 make(float x, float y, float z, float w)
        {
            Vec4 v;
            v.x = x;
            v.y = y;
            v.z = z;
            v.w = w;
            return v;
        }

		float x;
		float y;
		float z;
		float w;
	
		operator Vector4() const;
		operator Vector3() const { return Vector3(x, y, z); }
		operator Vec3() const { Vec3 v; v.x = x; v.y = y; v.z = z; return v; }
	};

	__declspec(align(16)) class CRAZEMATH_EXP Vector4
	{
	public:
		Vector4();
		Vector4(float x, float y, float z, float w);
		Vector4(const Vector3& v3, float w = 1.0f);

		operator Vector3() const { return Vector3(vec); }
		operator Vec4() const { return v; }
		Vec4* operator->() { return &v; }
		const Vec4* operator->() const { return &v; }

		void operator*=(float f) { vec = _mm_mul_ps(_mm_set_ps1(f), vec); }
		void operator/=(float f) { vec = _mm_div_ps(vec, _mm_set_ps1(f)); }

		union
		{
			Vec4 v;
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};
			__m128 vec;
		};
	};
}
#pragma warning(pop)