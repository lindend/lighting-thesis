#pragma once

#include "Export.h"

namespace Craze
{
	class CRAZEMATH_EXP EulerAngle
	{
	public:
		EulerAngle() { x = y = z = 0.0f; }
		EulerAngle(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
		//Rotation around the axes
		float x, y, z;

		void RotateX(float rotation) { x += rotation; }
		void RotateY(float rotation) { y += rotation; }
		void RotateZ(float rotation) { z += rotation; }

		void Set(float rotX, float rotY, float rotZ) { x = rotX; y = rotY; z = rotZ; }


	};
}

