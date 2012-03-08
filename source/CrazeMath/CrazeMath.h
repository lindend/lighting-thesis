#pragma once

#include "Export.h"


namespace Craze
{
	class Vector2;
	class Vector3;
	class Vector4;
	class Quaternion;
	class Matrix3;
	class Matrix4;

	const float FDELTA = 0.0001f;
	const float E = 2.718281f;
	const float PI = 3.14159f;

	CRAZEMATH_EXP float Sqrt(float num);
	CRAZEMATH_EXP float RSqrt(float num);
	 
	CRAZEMATH_EXP float Sin(float num);
	CRAZEMATH_EXP float Cos(float num);
	CRAZEMATH_EXP float Tan(float num);
	 
	CRAZEMATH_EXP float Abs(float num);
	 
	CRAZEMATH_EXP float Asin(float num);
	CRAZEMATH_EXP float Acos(float num);
	CRAZEMATH_EXP float Atan(float num);
	 
	CRAZEMATH_EXP float Lerp(float a, float b, float progress);
	 
	CRAZEMATH_EXP float Cot(float num);
	 
	/*CRAZEMATH_EXP float Max(float a, float b);
	CRAZEMATH_EXP float Min(float a, float b);*/

	CRAZEMATH_EXP bool FloatCmp(float a, float b);

	template <typename T> T Max(T a, T b) { return a > b ? a : b; }
	template <typename T> T Min(T a, T b) { return a < b ? a : b; }
	 
	CRAZEMATH_EXP float Square(float a);
	 
	CRAZEMATH_EXP float Frac(float a);

	CRAZEMATH_EXP unsigned int ClosestPow2(unsigned int num);
}

#include "EulerAngle.h"
#include "Matrix2.h"
#include "Matrix3.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Vector2.h"