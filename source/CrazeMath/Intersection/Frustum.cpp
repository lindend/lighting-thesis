#include "Frustum.h"

using namespace Craze;

Frustum Frustum::CreateFromMatrix(const Matrix4& m)
{
	Frustum f;
	//Left
	f.m_Planes[0] = Vector4(m[1][4] + m[1][1],
							m[2][4] + m[2][1],
							m[3][4] + m[3][1],
							m[4][4] + m[4][1]);

	//Right
	f.m_Planes[1] = Vector4(m[1][4] - m[1][1],
							m[2][4] - m[2][1],
							m[3][4] - m[3][1],
							m[4][4] - m[4][1]);
	//Bottom
	f.m_Planes[2] = Vector4(m[1][4] + m[1][2],
							m[2][4] + m[2][2],
							m[3][4] + m[3][2],
							m[4][4] + m[4][2]);

	//Top
	f.m_Planes[3] = Vector4(m[1][4] - m[1][2],
							m[2][4] - m[2][2],
							m[3][4] - m[3][2],
							m[4][4] - m[4][2]);

	//Near
	f.m_Planes[4] = Vector4(m[1][3],
							m[2][3],
							m[3][3],
							m[4][3]);

	//Far
	f.m_Planes[5] = Vector4(m[1][4] - m[1][3],
							m[2][4] - m[2][3],
							m[3][4] - m[3][3],
							m[4][4] - m[4][3]);

	return f;
}