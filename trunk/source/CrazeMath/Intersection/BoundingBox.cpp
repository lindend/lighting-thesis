#include "BoundingBox.h"

using namespace Craze;

void BoundingBox::IncludePoint(const Vector3& p)
{
	m_Min = Vector3::Min(p, m_Min);
	m_Max = Vector3::Max(p, m_Max);
}