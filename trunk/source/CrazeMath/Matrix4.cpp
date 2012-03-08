#include "Matrix4.h"
#include "CrazeMath.h"

#include "d3dx9math.h"

using namespace Craze;

D3DXMATRIX* ToD3DX(Matrix4* pM)
{
	return (D3DXMATRIX*)pM->m_Matrix[0];
}

const D3DXMATRIX* ToD3DX(const Matrix4* pM)
{
	return (D3DXMATRIX*)pM->m_Matrix[0];
}

const Matrix4 Matrix4::IDENTITY = Matrix4(1.0f, 0.0f, 0.0f, 0.0f,
										  0.0f, 1.0f, 0.0f, 0.0f,
										  0.0f, 0.0f, 1.0f, 0.0f,
										  0.0f, 0.0f, 0.0f, 1.0f);

Matrix4::Matrix4(float _00, float _01, float _02, float _03, 
				 float _10, float _11, float _12, float _13, 
				 float _20, float _21, float _22, float _23, 
				 float _30, float _31, float _32, float _33)
{
	m0 = _mm_setr_ps(_00, _01, _02, _03);
	m1 = _mm_setr_ps(_10, _11, _12, _13);
	m2 = _mm_setr_ps(_20, _21, _22, _23);
	m3 = _mm_setr_ps(_30, _31, _32, _33);
}

void Craze::Matrix4::SetToIdentity()
{
	*this = IDENTITY;
}

Craze::Matrix4 Craze::Matrix4::operator*(const Craze::Matrix4 &other) const
{
#define SPLAT(v, idx) _mm_shuffle_ps(v, v, _MM_SHUFFLE(idx, idx, idx, idx))
#define MULROW(tr, or) _mm_mul_ps(SPLAT(m ## tr, or), other.m ## or)

	Matrix4 res;
	res.m0 = _mm_add_ps(_mm_add_ps(MULROW(0, 0), MULROW(0, 1)), _mm_add_ps(MULROW(0, 2), MULROW(0, 3)));
	res.m1 = _mm_add_ps(_mm_add_ps(MULROW(1, 0), MULROW(1, 1)), _mm_add_ps(MULROW(1, 2), MULROW(1, 3)));
	res.m2 = _mm_add_ps(_mm_add_ps(MULROW(2, 0), MULROW(2, 1)), _mm_add_ps(MULROW(2, 2), MULROW(2, 3)));
	res.m3 = _mm_add_ps(_mm_add_ps(MULROW(3, 0), MULROW(3, 1)), _mm_add_ps(MULROW(3, 2), MULROW(3, 3)));

	return res;
}

float* Craze::Matrix4::operator [](unsigned int row)
{
	return m_Matrix[row];
}
const float* Craze::Matrix4::operator [](unsigned int row) const
{
	return m_Matrix[row];
}

Craze::Vector3 Craze::operator*(const Craze::Vector3& vec, const Craze::Matrix4& mat)
{
	Craze::Vector3 v;
	v.vec = _mm_add_ps(
				_mm_add_ps(	_mm_mul_ps(_mm_set_ps1(vec->x), mat.m0),
							_mm_mul_ps(_mm_set_ps1(vec->y), mat.m1)),
				_mm_add_ps(	_mm_mul_ps(_mm_set_ps1(vec->z), mat.m2),
							mat.m3));
	
	v.vec.m128_f32[3] = 0.f;

	return v;
}

Craze::Vector4 Craze::operator*(const Craze::Vector4& vec, const Craze::Matrix4& mat)
{
	__declspec(align(16)) Craze::Vector4 v(vec);
	
	TransformV4(mat, &v, 1);

	return v;
}

void Craze::Matrix4::Scale(float factor)
{
	Craze::Matrix4 m = IDENTITY;

	m[0][0] = factor;
	m[1][1] = factor;
	m[2][2] = factor;

	*this = m * (*this);
}

void Craze::Matrix4::Translate(const Craze::Vector3& pos)
{
	m3 = _mm_add_ps(m3, pos.vec);
}

void Craze::Matrix4::Rotate(const Craze::EulerAngle &angle)
{
	Craze::Matrix4 rot;

#ifdef CRAZE_DX9
	D3DXMatrixRotationYawPitchRoll(ToD3DX(this), angle.x, angle.y, angle.z);
#else
	rot[0][0] = Craze::Cos(angle.y)*Craze::Cos(angle.z) + Craze::Sin(angle.y)*Craze::Sin(angle.x)*Craze::Sin(angle.z);
	rot[0][1] = -Craze::Cos(angle.y)*Craze::Sin(angle.z) + Craze::Sin(angle.y)*Craze::Sin(angle.x)*Craze::Cos(angle.z);
	rot[0][2] = Craze::Sin(angle.y) * Craze::Cos(angle.x);
	rot[0][3] = 0.0f;
	rot[1][0] = Craze::Sin(angle.z) * Craze::Cos(angle.x);
	rot[1][1] = Craze::Cos(angle.z) * Craze::Cos(angle.x);
	rot[1][2] = -Craze::Sin(angle.x);
	rot[1][3] = 0.0f;
	rot[2][0] = -Craze::Sin(angle.y)*Craze::Cos(angle.z) + Craze::Cos(angle.y)*Craze::Sin(angle.x)*Craze::Sin(angle.z);
	rot[2][1] = Craze::Sin(angle.y)*Craze::Sin(angle.z) + Craze::Cos(angle.y)*Craze::Sin(angle.x)*Craze::Cos(angle.z);
	rot[2][2] = Craze::Cos(angle.y) * Craze::Cos(angle.x);
	rot[2][3] = 0.0f;
	rot[3][0] = 0.0f;
	rot[3][1] = 0.0f;
	rot[3][2] = 0.0f;
	rot[3][3] = 1.0f;
#endif

	*this = rot * (*this);
}

Craze::Vector3 Craze::Matrix4::GetTranslation() const
{
	Vector3 res(m3);
	res.vec.m128_f32[3] = 0;
	return res;
}
void Matrix4::SetTranslation(const Vector3& v)
{
	m3 = v.vec;
	m3.m128_f32[3] = 1.f;
}

Matrix4 Matrix4::GetInverse() const
{
	Matrix4 inv;
	float det = Determinant();
	D3DXMatrixInverse(ToD3DX(&inv), &det, ToD3DX(this));
	return inv;
}


float Matrix4::Determinant() const
{
	return D3DXMatrixDeterminant(ToD3DX(this));
	/*
	return 
		m_Matrix[0][0] * m_Matrix[1][1] * m_Matrix[2][2] * m_Matrix[3][3] +
		m_Matrix[0][0] * m_Matrix[1][2] * m_Matrix[2][3] * m_Matrix[3][1] +
		m_Matrix[0][0] * m_Matrix[1][3] * m_Matrix[2][1] * m_Matrix[3][2] +
		m_Matrix[0][1] * m_Matrix[1][0] * m_Matrix[2][3] * m_Matrix[3][2] +
		m_Matrix[0][1] * m_Matrix[1][2] * m_Matrix[2][0] * m_Matrix[3][3] +
		m_Matrix[0][1] * m_Matrix[1][3] * m_Matrix[2][2] * m_Matrix[3][0] +
		m_Matrix[0][2] * m_Matrix[1][0] * m_Matrix[2][1] * m_Matrix[3][3] +
		m_Matrix[0][2] * m_Matrix[1][1] * m_Matrix[2][3] * m_Matrix[3][0] +
		m_Matrix[0][2] * m_Matrix[1][3] * m_Matrix[2][0] * m_Matrix[3][1] +
		m_Matrix[0][3] * m_Matrix[1][0] * m_Matrix[2][2] * m_Matrix[3][1] +
		m_Matrix[0][3] * m_Matrix[1][1] * m_Matrix[2][0] * m_Matrix[3][2] +
		m_Matrix[0][3] * m_Matrix[1][2] * m_Matrix[2][1] * m_Matrix[3][0] +
		m_Matrix[0][0] * m_Matrix[1][1] * m_Matrix[2][3] * m_Matrix[3][2] -
		m_Matrix[0][0] * m_Matrix[1][2] * m_Matrix[2][1] * m_Matrix[3][3] -
		m_Matrix[0][0] * m_Matrix[1][3] * m_Matrix[2][2] * m_Matrix[3][1] -
		m_Matrix[0][1] * m_Matrix[1][0] * m_Matrix[2][2] * m_Matrix[3][3] -
		m_Matrix[0][1] * m_Matrix[1][2] * m_Matrix[2][3] * m_Matrix[3][0] -
		m_Matrix[0][1] * m_Matrix[1][3] * m_Matrix[2][0] * m_Matrix[3][2] -
		m_Matrix[0][2] * m_Matrix[1][0] * m_Matrix[2][3] * m_Matrix[3][1] -
		m_Matrix[0][2] * m_Matrix[1][1] * m_Matrix[2][0] * m_Matrix[3][3] -
		m_Matrix[0][2] * m_Matrix[1][3] * m_Matrix[2][1] * m_Matrix[3][0] -
		m_Matrix[0][3] * m_Matrix[1][0] * m_Matrix[2][1] * m_Matrix[3][2] -
		m_Matrix[0][3] * m_Matrix[1][1] * m_Matrix[2][2] * m_Matrix[3][0] -
		m_Matrix[0][3] * m_Matrix[1][2] * m_Matrix[2][0] * m_Matrix[3][1];
*/
}

Matrix4 Matrix4::GetTranspose() const
{
	return Matrix4(
		m_Matrix[0][0], m_Matrix[1][0], m_Matrix[2][0], m_Matrix[3][0],
		m_Matrix[0][1], m_Matrix[1][1], m_Matrix[2][1], m_Matrix[3][1],
		m_Matrix[0][2], m_Matrix[1][2], m_Matrix[2][2], m_Matrix[3][2],
		m_Matrix[0][3], m_Matrix[1][3], m_Matrix[2][3], m_Matrix[3][3]);
}

Matrix4 Matrix4::CreateScale(float x, float y, float z)
{
	return Matrix4( x, 0.0f, 0.0f, 0.0f,
					0.0f, y, 0.0f, 0.0f,
					0.0f, 0.0f, z, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::CreateTranslation(float x, float y, float z)
{
	return Matrix4(	1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x, y, z, 1.0f);
}

Matrix4 Matrix4::CreateTranslation(const Vector3& vec)
{
	Matrix4 m = Matrix4::IDENTITY;
	m.m3 = vec.vec;
	m.m3.m128_f32[3] = 1.f;
	return m;
}

Matrix4 Matrix4::CreateRotation(const Quaternion& q)
{
	/*m_Matrix[0][0] = 1 - 2 * q.y - 2 * q.z;
	m_Matrix[1][0] = 2 * (q.x * q.y + q.w * q.z);
	m_Matrix[2][0] = 2 * (q.x * q.z - q.w * q.y);
	m_Matrix[3][0] = 0.0f;

	m_Matrix[0][1] = 2 * (q.x * q.y - q.w * q.z);
	m_Matrix[1][1] = 1 - 2 * q.x - 2 * q.z;
	m_Matrix[2][1] = 2 * (q.z * q.y + q.w * q.x);
	m_Matrix[3][1] = 0.0f;

	m_Matrix[0][2] = 2 * (q.x * q.z + q.w * q.y);
	m_Matrix[1][2] = 2 * (q.z * q.y - q.w * q.x);
	m_Matrix[2][2] = 1 - 2 * q.x - 2 * q.y;
	m_Matrix[3][2] = 0.0f;

	m_Matrix[0][3] = 0.0f;
	m_Matrix[1][3] = 0.0f;
	m_Matrix[2][3] = 0.0f;
	m_Matrix[3][3] = 1.0f;*/

	return Matrix4(	1 - 2 * q.y - 2 * q.z,			2 * (q.x * q.y - q.w * q.z),	2 * (q.x * q.z + q.w * q.y),	0.0f,
					2 * (q.x * q.y + q.w * q.z),	1 - 2 * q.x - 2 * q.z,			2 * (q.z * q.y - q.w * q.x),	0.0f,
					2 * (q.x * q.z - q.w * q.y),	2 * (q.z * q.y + q.w * q.x),	1 - 2 * q.x - 2 * q.y,			0.0f,
					0.0f,							0.0f,							0.0f,							1.0f);

	
}

Matrix4 Matrix4::CreateFromRightUpForward(const Vector3& right, const Vector3& up, const Vector3& forward)
{
	Matrix4 result = IDENTITY;
	result.m0 = right.vec;
	result.m1 = up.vec;
	result.m2 = forward.vec;
	result.m3 = _mm_setr_ps(0.f, 0.f, 0.f, 1.f);

	return result;
}

Matrix4 Matrix4::CreatePerspectiveFov(float fovY, float aspect, float znear, float zfar)
{
	Matrix4 m = Matrix4::IDENTITY;
	m[0][0] = Craze::Cot(fovY / 2.0f) / aspect;
	m[1][1] = Craze::Cot(fovY / 2.0f);
	m[2][2] = zfar / (zfar - znear);
	m[2][3] = 1.0f;
	m[3][2] = -znear * zfar / (zfar - znear);
	m[3][3] = 0.0f;

	return m;
}

Matrix4 Matrix4::CreateView(const Vector3& eyePos, const Vector3& lookAt, const Vector3& up)
{
	Vector3 zaxis = Vector3::Normalize(lookAt - eyePos);

	Vector3 xaxis = Vector3::Normalize(up.Cross(zaxis));
	
	Vector3 yaxis = Vector3::Normalize(zaxis.Cross(xaxis));

	Matrix4 m;
	m.m0 = _mm_setr_ps(xaxis->x, yaxis->x, zaxis->x, 0.f);
	m.m1 = _mm_setr_ps(xaxis->y, yaxis->y, zaxis->y, 0.f);
	m.m2 = _mm_setr_ps(xaxis->z, yaxis->z, zaxis->z, 0.f);
	m.m3 = _mm_setr_ps(-xaxis.Dot(eyePos), -yaxis.Dot(eyePos), -zaxis.Dot(eyePos), 1.f);

	return m;
}

Matrix4 Matrix4::CreateOrtho(float width, float height, float znear, float zfar)
{
	Matrix4 m = IDENTITY;

	m[0][0] = 2.0f / width;
	m[1][1] = 2.0f / height;
	m[2][2] = 1.0f / (zfar - znear);
	m[3][2] = znear / (znear - zfar);

	return m;

}

void Craze::Transform(const Matrix4& m, Vector3* vs, unsigned int num)
{
	for (unsigned int i = 0; i < num; ++i)
	{
		__m128 vx = _mm_load_ps1(&vs[i]->x);
		__m128 vy = _mm_load_ps1(&vs[i]->y);
		__m128 vz = _mm_load_ps1(&vs[i]->z);

		vx = _mm_mul_ps(m.m0, vx);
		vy = _mm_mul_ps(m.m1, vy);
		vz = _mm_mul_ps(m.m2, vz);

		vs[i].vec = _mm_add_ps(_mm_add_ps(vx, vy), _mm_add_ps(vz, m.m3));
	}
}

void Craze::TransformV4(const Matrix4& m, Vector4* vs, unsigned int num)
{
	for (unsigned int i = 0; i < num; ++i)
	{
		__m128 vx = _mm_load_ps1(&vs[i]->x);
		__m128 vy = _mm_load_ps1(&vs[i]->y);
		__m128 vz = _mm_load_ps1(&vs[i]->z);
		__m128 vw = _mm_load_ps1(&vs[i]->w);

		vx = _mm_mul_ps(m.m0, vx);
		vy = _mm_mul_ps(m.m1, vy);
		vz = _mm_mul_ps(m.m2, vz);
		vw = _mm_mul_ps(m.m3, vw);

		vs[i].vec = _mm_add_ps(_mm_add_ps(vx, vy), _mm_add_ps(vz, vw));
	}
}

#define SPLAT(v, idx) _mm_shuffle_ps(v, v, _MM_SHUFFLE(idx, idx, idx, idx))

void Craze::Transform(const Matrix4& m, SoAV3* vs, unsigned int num)
{
	for (unsigned int i = 0; i < num; ++i)
	{

		__m128 tx = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 0)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 0))),
								_mm_add_ps(	_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 0)),
																 SPLAT(m.m3, 0)));
																	   
		__m128 ty = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 1)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 1))),
								_mm_add_ps(	_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 1)),
																 SPLAT(m.m3, 1)));
																	   
		vs[i].zs = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 2)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 2))),
								_mm_add_ps(	_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 2)),
																 SPLAT(m.m3, 2)));
		vs[i].xs = tx;
		vs[i].ys = ty;
	}
}

void TransformVecs(const Matrix4& m, SoAV3* vs, unsigned int num)
{
	for (unsigned int i = 0; i < num; ++i)
	{

		__m128 tx = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 0)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 0))),
											_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 0)));
																	   
		__m128 ty = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 1)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 1))),
											_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 1)));
																	   
		vs[i].zs = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 2)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 2))),
											_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 2)));
		vs[i].xs = tx;
		vs[i].ys = ty;
	}
}

Vector3 Craze::TransformVec(const Matrix4& m, const Vector3& v)
{
	__m128 vx = _mm_load_ps1(&v->x);
	__m128 vy = _mm_load_ps1(&v->y);
	__m128 vz = _mm_load_ps1(&v->z);

	vx = _mm_mul_ps(m.m0, vx);
	vy = _mm_mul_ps(m.m1, vy);
	vz = _mm_mul_ps(m.m2, vz);

	return Vector3(_mm_add_ps(_mm_add_ps(vx, vy), vz));
}

void Craze::TransformV4(const Matrix4& m, SoAV4* vs, unsigned int num)
{	
	for (unsigned int i = 0; i < num; ++i)
	{

		__m128 tx = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 0)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 0))),
								_mm_add_ps(	_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 0)),
											_mm_mul_ps(vs[i].ws, SPLAT(m.m3, 0))));
																	   
		__m128 ty = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 1)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 1))),
								_mm_add_ps(	_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 1)),
											_mm_mul_ps(vs[i].ws, SPLAT(m.m3, 1))));
																	   
		__m128 tz = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 2)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 2))),
								_mm_add_ps(	_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 2)),
											_mm_mul_ps(vs[i].ws, SPLAT(m.m3, 2))));
																	   
		vs[i].ws = _mm_add_ps(	_mm_add_ps(	_mm_mul_ps(vs[i].xs, SPLAT(m.m0, 3)),
											_mm_mul_ps(vs[i].ys, SPLAT(m.m1, 3))),
								_mm_add_ps(	_mm_mul_ps(vs[i].zs, SPLAT(m.m2, 3)),
											_mm_mul_ps(vs[i].ws, SPLAT(m.m3, 3))));

		vs[i].xs = tx;
		vs[i].ys = ty;
		vs[i].zs = tz;
	}
}

#undef SPLAT