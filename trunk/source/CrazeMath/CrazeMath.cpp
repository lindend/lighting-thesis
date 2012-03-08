#include "CrazeMath.h"

#include <math.h>
#include "mmintrin.h"
#include "xmmintrin.h"

using namespace Craze;

float Craze::RSqrt(float num)
{
	return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(num)));
}

float Craze::Sqrt(float num)
{
	return num * RSqrt(num);//sqrt(num);
}

float Craze::Square(float a)
{
    return a * a;
}

float Craze::Sin(float num)
{
	return sin(num);
}

float Craze::Cos(float num)
{
	return cos(num);
}

float Craze::Tan(float num)
{
	return tan(num);
}

float Craze::Asin(float num)
{
	return asin(num);
}

float Craze::Acos(float num)
{
	return acos(num);
}

float Craze::Atan(float num)
{
	return atan(num);
}

float Craze::Abs(float num)
{
	return abs(num);
}

float Craze::Cot(float num)
{
	float t = Craze::Tan(num);

	return t == 0.0f ? 0.0f : (1.0f / t);

	/*if (t == 0.0f)
		return 0.0f;

	return 1.0f / t;*/
}

float Craze::Lerp(float a, float b, float progress)
{
	return (a + (b - a) * progress);
}

unsigned int Craze::ClosestPow2(unsigned int num)
{
	unsigned int i;
	for (i = 1; i <= num; i <<= 1);
	return i;
}
/*
float Craze::Max(float a, float b)
{
	return a > b ? a : b;
}

float Craze::Min(float a, float b)
{
	return a < b ? a : b;
}*/

float Craze::Frac(float a)
{
	return a - (int)a;
}

bool Craze::FloatCmp(float a, float b)
{
	return Abs(a - b) < FDELTA;
}