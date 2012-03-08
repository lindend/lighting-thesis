#pragma once
#include "../Export.h"

#include <limits>

#include "Memory/MemoryManager.h"

#include "../Vector3.h"
#include "../V3Math.h"

namespace Craze
{
	class CRAZEMATH_EXP BoundingBox
	{
		CRAZE_ALLOC_ALIGN(16);
	public:
		BoundingBox()
		{
			float max = std::numeric_limits<float>::max();
			m_Min.Set(max, max, max);
			m_Max.Set(-max, -max, -max);
		}

		Vector3 m_Min;
		Vector3 m_Max;

		void IncludePoint(const Vector3& point);
		void Include(const BoundingBox& bb)
		{
		    m_Min = Min(m_Min, bb.m_Min);
		    m_Max = Max(m_Max, bb.m_Max);
		}

	};
}
