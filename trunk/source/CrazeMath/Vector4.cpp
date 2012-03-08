#include "Vector4.h"

using namespace Craze;

Vec4::operator Vector4() const
{ 
	Vector4 v; 
	v.v = *this; 
	return v; 
}

Vector4::Vector4()
{

}

Vector4::Vector4(float _x, float _y, float _z, float _w)
{
	vec = _mm_setr_ps(_x, _y, _z, _w);
}

Vector4::Vector4(const Vector3& v3, float _w)
{
	vec = _mm_setr_ps(v3->x, v3->y, v3->z, _w);
}
