#include "Vector3.h"
#include "CrazeMath.h"

using namespace Craze;

CRAZE_POOL_ALLOC_ALIGN_IMPL(Vector3, 16);

const Vector3 Vector3::UP = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::DOWN = Vector3(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::LEFT = Vector3(-1.0f, 0.0f, 0.0f);
const Vector3 Vector3::RIGHT = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::FORWARD = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::BACKWARD = Vector3(0.0f, 0.0f, -1.0f);

const Vector3 Vector3::ZERO = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::ONE = Vector3(1.0f, 1.0f, 1.0f);

Vec3 Craze::MkV3(float _x, float _y, float _z) { Vec3 v; v.x = _x; v.y = _y; v.z = _z; return v; }

Vec3::operator Vector3() const
{
	return Vector3(x, y, z);
}

Vec3::operator Vector4() const
{
	return Vector4(x, y, z, 1.f);
}

Vec3::operator Vec4() const
{
	Vec4 v = { x, y, z, 1.f };
	return v;
}

Craze::Vector3::Vector3( float _x, float _y, float _z )
{
	vec = _mm_setr_ps(_x, _y, _z, 0.0f);
}

void Craze::Vector3::Set( float _x, float _y, float _z )
{
	vec = _mm_setr_ps(_x, _y, _z, 0.0f);
}

float Craze::Vector3::GetLen() const
{
	return Sqrt(GetSquaredLen());
}

float Craze::Vector3::GetRLen() const
{
	return RSqrt(GetSquaredLen());
}

float Craze::Vector3::GetSquaredLen() const
{
	__m128 sq = _mm_mul_ps(vec, vec);
	return sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
}
void Craze::Vector3::Normalize()
{
	float rlen = GetRLen();

	*this *= rlen;
}

Vector3 Vector3::Normalized() const
{
	return Normalize(*this);
}

Vector3 Vector3::Normalize(const Craze::Vector3 &o)
{
	Vector3 ret = o;
	ret.Normalize();
	return ret;
}

Craze::Vector3 Craze::Vector3::Cross(const Vector3& o) const
{
	//y * v.z - z * v.y
	//z * v.x - x * v.z
	//x * v.y - y * v.x

#define YZXSHUF(vect) _mm_shuffle_ps(vect, vect, _MM_SHUFFLE(3, 0, 2, 1))
#define ZXYSHUF(vect) _mm_shuffle_ps(vect, vect, _MM_SHUFFLE(3, 1, 0, 2))

	Vector3 v(_mm_sub_ps(_mm_mul_ps(YZXSHUF(vec), ZXYSHUF(o.vec)), _mm_mul_ps(ZXYSHUF(vec), YZXSHUF(o.vec))));
	v.vec.m128_f32[3] = 0.f;
	return v;

#undef YZXSHUF
#undef ZXYSHUF
}

float Craze::Vector3::Dot(const Vector3& o) const
{
	__m128 p = _mm_mul_ps(vec, o.vec);
	return (p.m128_f32[0] + p.m128_f32[1] + p.m128_f32[2]);
}

Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2)
{
	return Vector3(_mm_max_ps(v1.vec, v2.vec));
}

Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2)
{
	return Vector3(_mm_min_ps(v1.vec, v2.vec));
}