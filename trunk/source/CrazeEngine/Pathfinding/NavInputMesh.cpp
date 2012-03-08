#include "CrazeEngine.h"
#include "NavInputMesh.h"

using namespace Craze;

CRAZE_POOL_ALLOC_IMPL(NavInputMesh);

void NavInputMesh::Set(const Vec3* pVerts, int stride, int numVerts, const int* pIndices, int numTris)
{
	Free();
	SetVerts(pVerts, stride, numVerts);
	
	m_NumTris = numTris;
	m_pIndices = (int*)gMemory.Allocate(sizeof(int) *  numTris * 3, __FILE__, __LINE__);
	memcpy(m_pIndices, pIndices, sizeof(int) * numTris * 3);
}

void NavInputMesh::Set(const Vec3* pVerts, int stride, int numVerts, const unsigned short* pIndices, int numTris)
{
	Free();
	SetVerts(pVerts, stride, numVerts);

	m_NumTris = numTris;
	m_pIndices = (int*)gMemory.Allocate(sizeof(int) *  numTris * 3, __FILE__, __LINE__);

	for (int i = 0; i < numTris; ++i)
	{
		const int idx = i * 3;
		m_pIndices[idx + 0] = pIndices[idx + 0];
		m_pIndices[idx + 1] = pIndices[idx + 1];
		m_pIndices[idx + 2] = pIndices[idx + 2];
	}
}

void NavInputMesh::SetVerts(const Vec3* pVerts, int stride, int numVerts)
{
	m_pBB = CrNew BoundingBox();
	m_pVertices = (Vec3* )gMemory.Allocate(sizeof(Vec3) *  numVerts, __FILE__, __LINE__);

	m_NumVerts = numVerts;

	const char* pInput = (const char*)pVerts;

	for (int i = 0; i < numVerts; ++i)
	{
		m_pVertices[i] = *(const Vec3*)&pInput[i * stride];
		m_pBB->IncludePoint(m_pVertices[i]);
	}
}

void NavInputMesh::Free()
{
	gMemory.Free(m_pVertices);
	delete m_pBB;
}