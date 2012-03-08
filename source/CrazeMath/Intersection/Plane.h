#pragma once
#include "../Export.h"

#include "../Vector3.h"
#include "../Vector4.h"

namespace Craze
{
    class CRAZEMATH_EXP Plane
    {
    public:
        Plane() {}
		Plane(const Vector3& pos, const Vector3& normal) { m_Plane.vec = normal.vec; m_Plane->w = -pos.Dot(normal); }

        Vector4 m_Plane;
    };
}
