#include "Matrix2.h"
#include "CrazeMath.h"

using Craze::Vector2;
using Craze::Matrix2;

void Matrix2::SetToIdentity()
{
	m_Matrix[0][0] = 1.0f;
	m_Matrix[0][1] = 0.0f;
	m_Matrix[1][0] = 0.0f;
	m_Matrix[1][1] = 1.0f;
}

Matrix2 Matrix2::operator* (const Matrix2& m) const
{
	Matrix2 mat;
	mat[0][0] = m_Matrix[0][0] * m[0][0] + m_Matrix[0][1] * m[1][0];
	mat[0][1] = m_Matrix[0][0] * m[0][1] + m_Matrix[0][1] * m[1][1];
	mat[1][0] = m_Matrix[1][0] * m[0][0] + m_Matrix[1][1] * m[1][0];
	mat[1][1] = m_Matrix[1][0] * m[0][1] + m_Matrix[1][1] * m[1][1];

	return mat;
}

float* Matrix2::operator[](unsigned int row) const
{
	return (float*)m_Matrix[row];
}

Vector2 Craze::operator*(const Vector2& v, const Matrix2& m)
{
	return Vector2(v.x * m[0][0] + v.y * m[1][0], v.x * m[1][0] + v.y * m[1][1]);
}