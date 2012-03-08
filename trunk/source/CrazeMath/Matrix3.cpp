#include "Matrix3.h"
#include "Matrix4.h"
#include "Vector3.h"

using namespace Craze;

Matrix3::Matrix3(const Matrix4& m)
{
	m_Mat[0][0] = m.m_Matrix[0][0];
	m_Mat[0][1] = m.m_Matrix[0][1];
	m_Mat[0][2] = m.m_Matrix[0][2];

	m_Mat[1][0] = m.m_Matrix[1][0];
	m_Mat[1][1] = m.m_Matrix[1][1];
	m_Mat[1][2] = m.m_Matrix[1][2];

	m_Mat[2][0] = m.m_Matrix[2][0];
	m_Mat[2][1] = m.m_Matrix[2][1];
	m_Mat[2][2] = m.m_Matrix[2][2];
}

Matrix3 Craze::Transpose(const Matrix3& m)
{
	Matrix3 res;
	res.m_Mat[0][0] = m.m_Mat[0][0];
	res.m_Mat[0][1] = m.m_Mat[1][0];
	res.m_Mat[0][2] = m.m_Mat[2][0];
								 
	res.m_Mat[1][0] = m.m_Mat[0][1];
	res.m_Mat[1][1] = m.m_Mat[1][1];
	res.m_Mat[1][2] = m.m_Mat[2][1];
							 
	res.m_Mat[2][0] = m.m_Mat[0][2];
	res.m_Mat[2][1] = m.m_Mat[1][2];
	res.m_Mat[2][2] = m.m_Mat[2][2];
	
	return res;
}

Matrix3 Craze::Inverse(const Matrix3& m)
{
	Vector3 r0(m.m_Mat[0][0], m.m_Mat[0][1], m.m_Mat[0][2]);
	Vector3 r1(m.m_Mat[1][0], m.m_Mat[1][1], m.m_Mat[1][2]);
	Vector3 r2(m.m_Mat[2][0], m.m_Mat[2][1], m.m_Mat[2][2]);
	
	float det = Determinant(m);

	Vector3 res0 = r1.Cross(r2) / det;
	Vector3 res1 = r2.Cross(r0) / det;
	Vector3 res2 = r0.Cross(r1) / det;

	Matrix3 res;
	res.m_Mat[0][0] = res0.v.x;
	res.m_Mat[1][0] = res0.v.y;
	res.m_Mat[2][0] = res0.v.z;

	res.m_Mat[0][1] = res1.v.x;
	res.m_Mat[1][1] = res1.v.y;
	res.m_Mat[2][1] = res1.v.z;

	res.m_Mat[0][2] = res2.v.x;
	res.m_Mat[1][2] = res2.v.y;
	res.m_Mat[2][2] = res2.v.z;

	return res;
}

float Craze::Determinant(const Matrix3& m)
{
	return	m.m_Mat[0][0] * m.m_Mat[1][1] * m.m_Mat[2][2] +
			m.m_Mat[0][1] * m.m_Mat[1][2] * m.m_Mat[2][0] + 
			m.m_Mat[0][2] * m.m_Mat[1][0] * m.m_Mat[2][1] -
			m.m_Mat[0][0] * m.m_Mat[1][2] * m.m_Mat[2][1] - 
			m.m_Mat[0][1] * m.m_Mat[1][0] * m.m_Mat[2][2] -
			m.m_Mat[0][2] * m.m_Mat[1][1] * m.m_Mat[2][0];
}