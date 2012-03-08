#include "CrazeGraphicsPCH.h"
#include "MeshTools.h"

#include "VertexStreams.h"

using namespace Craze;
using namespace Craze::Graphics2;

struct Qmat
{
	float m[10];
};

struct Face
{
	unsigned short v0;
	unsigned short v1;
	unsigned short v2;
	
};

struct Vert
{
	Vector3 pos;
};

void calcNormals(const Face* pFs, unsigned int numFaces, const Vertex* pVerts, Vector3* pTarget)
{
	for (unsigned int i = 0; i < numFaces; ++i)
	{
		Vector3 v0 = pVerts[pFs[i].v0].position;
		Vector3 v1 = pVerts[pFs[i].v1].position;
		Vector3 v2 = pVerts[pFs[i].v2].position;

		pTarget[i] = (v1 - v0).Cross(v2 - v0).Normalized();
	}
}

inline float area(Vector3& v0, Vector3& v1, Vector3& v2)
{
	return 0.5f * (v1 - v0).Cross(v2 - v1).GetLen();
}

void calcQfs(Qmat* pOutQfs, unsigned int numFaces, const Vertex* pVerts, const Vector3* pNorms)
{
	for (unsigned int i = 0; i < numFaces; ++i)
	{

	}
}

MeshData* Craze::Graphics2::SimplifyMeshQEM(MeshData* pMesh)
{
	MeshData* d = 0;

	MEM_AUTO_MARK_STACK;

	unsigned int numFaces = pMesh->GetNumIndices() / 3;
	const Face* pFaces = (const Face*)pMesh->GetIndices();
	const Vertex* pVerts = (Vertex*)pMesh->GetPosNormalUv();

	Vector3* pNormals = (Vector3*)gMemory.StackAlloc(Align(16), sizeof(Vector3) * numFaces);
	calcNormals(pFaces, numFaces, pVerts, pNormals);

	Qmat* pQfs = (Qmat*)gMemory.StackAlloc(sizeof(Qmat) * numFaces);


	Qmat* pQvs = (Qmat*)gMemory.StackAlloc(sizeof(Qmat) * pMesh->GetNumVertices());

	
	return d;
}