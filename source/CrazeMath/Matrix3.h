#pragma once
#include "Export.h"

namespace Craze
{
	class Matrix4;

	class CRAZEMATH_EXP Matrix3
	{
	public:
		Matrix3() {}
		Matrix3(const Matrix4& m);

		float m_Mat[3][3];
	};

	CRAZEMATH_EXP Matrix3 Transpose(const Matrix3& m);
	CRAZEMATH_EXP Matrix3 Inverse(const Matrix3& m);
	CRAZEMATH_EXP float Determinant(const Matrix3& m);
}
