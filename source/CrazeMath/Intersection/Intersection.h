#pragma once

#include "../Export.h"

#include "Ray.h"
#include "Plane.h"
#include "BoundingBox.h"
#include "../Matrix4.h"
#include "Triangle.h"

namespace Craze
{
    struct CRAZEMATH_EXP RayTestResult
    {
		union
		{
			float results[4];
			__m128 res;
		};
    };

	struct CRAZEMATH_EXP RayBoxResult
	{
		float tMin;
		float tMax;
		bool intersects;
	};

	struct CRAZEMATH_EXP SoAPlane
	{
		__m128 a;
		__m128 b;
		__m128 c;
		__m128 d;
	};

    class CRAZEMATH_EXP Intersection
    {
    public:
        //Tests a ray versus four planes
        static RayTestResult Test(const Ray& r, Plane* pPlanes);
		static void Test(const Ray& r, SoAPlane* pPlanes, RayTestResult* pRes, unsigned int num);
		static RayBoxResult Test(const Ray& r, const BoundingBox& b);

		static float Test(const Ray& r, const Triangle& t);

		static bool IsInside(const Matrix4& vp, const BoundingBox& b, float* pOutDepth);
    };
}
