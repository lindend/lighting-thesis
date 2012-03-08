#include "Quaternion.h"

#include "CrazeMath.h"

using namespace Craze;

const Quaternion Quaternion::IDENTITY = Quaternion(0.f, 0.f, 0.f, 1.f);

void Quaternion::Normalize()
{
	float length = GetRLength();
	x *= length;
	y *= length;
	z *= length;
	w *= length;
}

Quaternion Quaternion::Normalized() const
{
	Quaternion q = *this;
	q.Normalize();
	return q;
}

Vector3 Quaternion::Transform(const Vector3& v) const
{
	Quaternion qInv = this->Conjugate();
	Quaternion p(v->x, v->y, v->z, 0.0f);
	return (qInv * p * *this).Im();
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

float Quaternion::GetLength() const
{
	return Sqrt(x * x + y * y + z * z + w * w);
}

float Quaternion::GetRLength() const
{
	return RSqrt(x * x + y * y + z * z + w * w);
}

float Quaternion::GetSquaredLength() const
{
	return x * x + y * y + z * z + w * w;
}

float Quaternion::Real() const
{
	return w;
}

Vector3 Quaternion::Im() const
{
	return Vector3(x, y, z);
}

/**
Initializes an identity quaternion
*/
Quaternion::Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

Quaternion::Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
{
}

Quaternion::Quaternion(float yaw, float pitch, float roll)
{
	Quaternion qx(Sin(yaw / 2.0f), 0.0f, 0.0f, Cos(yaw / 2.0f));
	Quaternion qy(0.0f, Sin(pitch / 2.0f), 0.0f, Cos(pitch / 2.0f));
	Quaternion qz(0.0f, Sin(roll / 2.0f), 0.0f, Cos(roll / 2.0f));

	*this = qx * qy * qz;
}

Quaternion::Quaternion(const Vector3& axis, float angle)
{
	float sinv = Sin(angle / 2.0f);

	w = Cos(angle / 2.0f);
	x = axis->x * sinv;
	y = axis->y * sinv;
	z = axis->z * sinv;
				
}

Quaternion::Quaternion(const Vector3& dirFrom, const Vector3& dirTo)
{
	/*vector3 c = cross(v1,v2);
	q.v = c;
	if ( vectors are known to be unit length ) {
		q.w = 1 + dot(v1,v2);
	} else {
		q.w = sqrt(v1.length_squared() * v2.length_squared()) + dot(v1,v2);
	}
	q.normalize();
	return q;*/

	Vector3 cr = dirFrom.Cross(dirTo);
	x = cr->x;
	y = cr->y;
	z = cr->z;
	w = 1 + dirFrom.Dot(dirTo);
	Normalize();
				
/*
	dirFrom.Normalize();
	dirTo.Normalize();

	float eSqrt = Sqrt(2 * (1 + dirFrom.Dot(dirTo)));
	Vector3 im =  dirFrom.Cross(dirTo) / eSqrt;
	x = im.x;
	y = im.y;
	z = im.z;
	w = eSqrt / 2.0f;*/
}

Quaternion Quaternion::operator*(const Quaternion& o) const
{
	Quaternion q;
	q.w = w * o.w - x * o.x - y * o.y - z * o.z;
	q.x = w * o.x + x * o.w + y * o.z - z * o.y;
	q.y = w * o.y + y * o.w + z * o.x - x * o.z;
	q.z = w * o.z + z * o.w + x * o.y - y * o.x;

	return q;
}

Quaternion Quaternion::operator/(float f) const
{
	return Quaternion(x / f, y / f, z / f, w / f);
}

Quaternion Quaternion::operator*(float f) const
{
	return Quaternion(x * f, y * f, z * f, w * f);
}