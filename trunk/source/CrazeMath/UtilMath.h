#pragma once

#include "V3Math.h"
#include "Intersection/BoundingBox.h"

namespace Craze
{

	inline Vector3 Clamp(const Vector3& v, const BoundingBox& bb)
	{
		return Min(Max(v, bb.m_Min), bb.m_Max);
	}
}