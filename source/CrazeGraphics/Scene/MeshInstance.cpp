#include "CrazeGraphicsPCH.h"
#include "MeshInstance.h"
#include "Scene.h"

using namespace Craze::Graphics2;
using namespace Craze;

void MeshInstance::SetTransform(const Matrix4& transform)
{
	m_RI.m_Transform = transform;
	*m_pBoundingBox = *m_RI.m_pMesh->m_pBoundingBox;
	Transform(transform, (Vector3*)m_pBoundingBox, 2);
}

void MeshInstance::Draw(DrawList* pDL, float depth)
{
	m_RI.m_Material = *m_RI.m_pMesh->GetMaterial();
	m_SortId.depth = (int)(depth * (2 << 24));
	pDL->push_back(std::make_pair(m_SortId.Encode(), &m_RI));
}

void MeshInstance::Remove()
{
	if (dynamic)
	{
		m_pScene->RemoveDynamicMesh(this);
	} else
	{
		m_pScene->RemoveStaticMesh(this);
	}
}