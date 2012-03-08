#pragma once
#include "../Export.h"

#include "BoundingBox.h"
#include "Plane.h"
#include "../Matrix4.h"

namespace Craze
{
		class CRAZEMATH_EXP Frustum
		{
		public:

			static Frustum CreateFromMatrix(const Matrix4& m);

			Vector4 m_Planes[6];
		};
}