#include "CrazeEngine.h"
#include "NavigationScene.h"

#include <algorithm>

#include "recast/Recast.h"
#include "detour/DetourNavMeshBuilder.h"
#include "detour/DetourNavMesh.h"
#include "detour/DetourNavMeshQuery.h"

#include "Mesh.h"
#include "Device.h"
#include "Material.h"

using namespace Craze;
using namespace Craze::Graphics2;

NavigationScene::NavigationScene() : m_pMesh(nullptr), m_pMeshDetail(nullptr), m_pNavMesh(nullptr)
{
	m_pNavQuery = dtAllocNavMeshQuery();
}

void NavigationScene::CleanUp()
{
	rcFreePolyMesh(m_pMesh);
	m_pMesh = nullptr;
	
	rcFreePolyMeshDetail(m_pMeshDetail);
	m_pMeshDetail = nullptr;

	dtFreeNavMesh(m_pNavMesh);
	m_pNavMesh = nullptr;

	dtFreeNavMeshQuery(m_pNavQuery);
	m_pNavQuery = nullptr;
}

void NavigationScene::AddMesh(const NavInputMesh* pMesh)
{
	m_pMeshes.push_back(pMesh);
}
const NavInputMesh* NavigationScene::GetMesh(const NavInputMesh* pMesh) const
{
	auto it = std::find(m_pMeshes.begin(), m_pMeshes.end(), pMesh);
	return it != m_pMeshes.end() ? *it : nullptr;
}
void NavigationScene::RemoveMesh(const NavInputMesh* pMesh)
{
	for (int i = 0; i < m_pMeshes.size(); ++i)
	{
		if (m_pMeshes[i] == pMesh)
		{
			m_pMeshes[i] = m_pMeshes.back();
			m_pMeshes.pop_back();
		}
	}
}

bool NavigationScene::FindPath(const Vec3& begin, const Vec3& end, Path* pOutPath)
{
	static const float pickExt[] = {2.f, 4.f, 2.f};
	static dtQueryFilter filter;
	filter.setIncludeFlags(1);
	filter.setExcludeFlags(0);

	dtPolyRef startRef;
	dtPolyRef endRef;
	if (dtStatusFailed(m_pNavQuery->findNearestPoly(&begin.x, pickExt, &filter, &startRef, 0)))
	{
		LOG_ERROR("Unable to find starting poly ref for path");
		return false;
	}

	if (dtStatusFailed(m_pNavQuery->findNearestPoly(&end.x, pickExt, &filter, &endRef, 0)))
	{
		LOG_ERROR("Unable to find end poly ref for path");
		return false;
	}

	const int MaxPolys = 512;
	dtPolyRef polys[MaxPolys];
	int numPolys = 0;
	if (dtStatusFailed(m_pNavQuery->findPath(startRef, endRef, &begin.x, &end.x, &filter, polys, &numPolys, Min<int>(MaxPolys, pOutPath->GetMaxVerts()))))
	{
		LOG_ERROR("Unable to find path");
		return false;
	}

	if (numPolys)
	{
		Vec3 closestEnd = end;
		if (polys[numPolys - 1] != endRef)
		{
			m_pNavQuery->closestPointOnPoly(polys[numPolys - 1], &end.x, &closestEnd.x);
		}
		int numVerts = 0;
		m_pNavQuery->findStraightPath(&begin.x, &closestEnd.x, polys, numPolys, (float*)pOutPath->GetVerts(), pOutPath->GetPathFlags(), nullptr, &numVerts, Min<int>(MaxPolys, pOutPath->GetMaxVerts()));
		pOutPath->SetNumVerts(numVerts);
		return true;
	}
	return false;
}

bool NavigationScene::Build()
{
	if (CreateRecastMeshes())
	{
		return CreateDetourMesh();
	}
	return false;
}

namespace Craze
{
	float AgentHeight = 1.8f;
	float AgentClimb = 0.7f;
	float AgentWidth = 0.4f;
	int MaxEdgeLen = 12;
	float MaxEdgeError = 1.3f;
	int MinRegionSize = 8;
	int MergedRegionSize = 20;
	int MaxVertsPerPoly = 6;
	float DetailSampleDist = 6.f;
	float CellSize = 0.2f;
	float CellHeight = 0.3f;
}

bool NavigationScene::CreateRecastMeshes()
{
	if (m_pMeshes.size() == 0)
	{
		return false;
	}
	rcConfig conf;
	conf.cs = CellSize;
	conf.ch = CellHeight;
	conf.walkableSlopeAngle = 45.f;
	conf.walkableHeight = (int)ceil(AgentHeight / conf.ch);
	conf.walkableClimb = (int)floor(AgentClimb / conf.ch);
	conf.walkableRadius = (int)ceil(AgentWidth / conf.cs);
	conf.maxEdgeLen = MaxEdgeLen / conf.cs;
	conf.maxSimplificationError = MaxEdgeError;
	conf.minRegionArea = MinRegionSize * MinRegionSize;
	conf.mergeRegionArea = MergedRegionSize * MergedRegionSize;
	conf.maxVertsPerPoly = Min<int>(DT_VERTS_PER_POLYGON, MaxVertsPerPoly);
	conf.detailSampleDist = DetailSampleDist * conf.cs;
	conf.detailSampleMaxError = conf.ch;

	BoundingBox bb;
	for (int i = 0; i < m_pMeshes.size(); ++i)
	{
		const BoundingBox* pBB = m_pMeshes[i]->GetBB();
		bb.IncludePoint(pBB->m_Min);
		bb.IncludePoint(pBB->m_Max);
	}
	rcVcopy(conf.bmin, &bb.m_Min.x);
	rcVcopy(conf.bmax, &bb.m_Max.x);

	rcCalcGridSize(conf.bmin, conf.bmax, conf.cs, &conf.width, &conf.height);

	rcHeightfield* pSolid = rcAllocHeightfield();

	rcContext ctx;

	if (!rcCreateHeightfield(&ctx, *pSolid, conf.width, conf.height, conf.bmin, conf.bmax, conf.cs, conf.ch))
	{
		LOG_ERROR("Unable to create heightfield");
		rcFreeHeightField(pSolid);
		CleanUp();
		return false;
	}

	int totalTris = 0;
	int totalVerts = 0;
	for (int i = 0; i < m_pMeshes.size(); ++i)
	{
		totalTris += m_pMeshes[i]->GetNumTris();
		totalVerts += m_pMeshes[i]->GetNumVerts();
	}

	unsigned char* pTriAreas = (unsigned char*)gMemory.Allocate(sizeof(unsigned char) * totalTris, __FILE__, __LINE__);
	memset(pTriAreas, 0, sizeof(unsigned char) * totalTris);

	int processedTris = 0;
	for (int i = 0; i < m_pMeshes.size(); ++i)
	{
		const float* pVerts = (const float*)m_pMeshes[i]->GetVertices();
		const int numVerts = m_pMeshes[i]->GetNumVerts();
		const int* pTris = m_pMeshes[i]->GetIndices();
		const int numTris = m_pMeshes[i]->GetNumTris();

		rcMarkWalkableTriangles(&ctx, conf.walkableSlopeAngle, pVerts, numVerts, pTris, numTris, pTriAreas + processedTris);
		rcRasterizeTriangles(&ctx, pVerts, numVerts, pTris, pTriAreas, numTris, *pSolid, conf.walkableClimb);
		processedTris += numTris;
	}

	gMemory.Free(pTriAreas);

	rcFilterLowHangingWalkableObstacles(&ctx, conf.walkableClimb, *pSolid);
	rcFilterLedgeSpans(&ctx, conf.walkableHeight, conf.walkableClimb, *pSolid);
	rcFilterWalkableLowHeightSpans(&ctx, conf.walkableHeight, *pSolid);

	rcCompactHeightfield* pChf = rcAllocCompactHeightfield();

	if (!rcBuildCompactHeightfield(&ctx, conf.walkableHeight, conf.walkableClimb, *pSolid, *pChf))
	{
		LOG_ERROR("Error while building navigation mesh: unable to build compact heightfield");
		rcFreeCompactHeightfield(pChf);
		return false;
	}

	if (!rcErodeWalkableArea(&ctx, conf.walkableRadius, *pChf))
	{
		LOG_ERROR("Error while building navigation mesh: unable to erode walkable area");
		rcFreeCompactHeightfield(pChf);
		return false;
	}

	if (!rcBuildRegionsMonotone(&ctx, *pChf, 0, conf.minRegionArea, conf.mergeRegionArea))
	{
		LOG_ERROR("Error while building navigation mesh: unable to create regions");
		rcFreeCompactHeightfield(pChf);
		return false;
	}

	rcContourSet* pCset = rcAllocContourSet();
	if (!rcBuildContours(&ctx, *pChf, conf.maxSimplificationError, conf.maxEdgeLen, *pCset))
	{
		LOG_ERROR("Error while building navigation mesh: unable to create contours");
		rcFreeCompactHeightfield(pChf);
		rcFreeContourSet(pCset);
		return false;
	}

	m_pMesh = rcAllocPolyMesh();
	if (!rcBuildPolyMesh(&ctx, *pCset, conf.maxVertsPerPoly, *m_pMesh))
	{
		LOG_ERROR("Error while building navigation mesh: unable to create mesh");
		rcFreeCompactHeightfield(pChf);
		rcFreeContourSet(pCset);
		rcFreePolyMesh(m_pMesh);
		m_pMesh = 0;
		return false;
	}

	m_pMeshDetail = rcAllocPolyMeshDetail();
	if (!rcBuildPolyMeshDetail(&ctx, *m_pMesh, *pChf, conf.detailSampleDist, conf.detailSampleMaxError, *m_pMeshDetail))
	{
		LOG_ERROR("Error while building navigation mesh: unable to create detail mesh");
		rcFreeCompactHeightfield(pChf);
		rcFreeContourSet(pCset);
		rcFreePolyMesh(m_pMesh);
		m_pMesh = 0;
		rcFreePolyMeshDetail(m_pMeshDetail);
		m_pMeshDetail = 0;
		return false;
	}

	rcFreeCompactHeightfield(pChf);
	rcFreeContourSet(pCset);

	return true;
}

bool NavigationScene::CreateDetourMesh()
{
	if (!(m_pMesh && m_pMeshDetail))
	{
		LOG_ERROR("Error while building navigation mesh: no recast meshes found");
		return false;
	}

	for (int i = 0; i < m_pMesh->npolys; ++i)
	{
		if (m_pMesh->areas[i] == RC_WALKABLE_AREA)
		{
			m_pMesh->areas[i] = 0;
			m_pMesh->flags[i] = 1;
		}
	}

	dtNavMeshCreateParams params;
	memset(&params, 0, sizeof(params));
	params.verts = m_pMesh->verts;
	params.vertCount = m_pMesh->nverts;
	params.polys = m_pMesh->polys;
	params.polyAreas = m_pMesh->areas;
	params.polyFlags = m_pMesh->flags;
	params.polyCount = m_pMesh->npolys;
	params.nvp = m_pMesh->nvp;
	params.detailMeshes = m_pMeshDetail->meshes;
	params.detailVerts = m_pMeshDetail->verts;
	params.detailVertsCount = m_pMeshDetail->nverts;
	params.detailTris = m_pMeshDetail->tris;
	params.detailTriCount = m_pMeshDetail->ntris;
	params.walkableHeight = AgentHeight;
	params.walkableRadius = AgentWidth;
	params.walkableClimb = AgentClimb;
	rcVcopy(params.bmin, m_pMesh->bmin);
	rcVcopy(params.bmax, m_pMesh->bmax);
	params.cs = CellSize;
	params.ch = CellHeight;
	params.buildBvTree = true;

	unsigned char* navData = 0;
	int navDataSize = 0;

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
	{
		LOG_ERROR("Error while building navigation mesh: unable to create detour navmesh");
		return false;
	}

	m_pNavMesh = dtAllocNavMesh();

	if (dtStatusFailed(m_pNavMesh->init(navData, navDataSize, DT_TILE_FREE_DATA)))
	{
		LOG_ERROR("Error while building navigation mesh: unable to init detour navmesh");
		dtFree(navData);
		dtFreeNavMesh(m_pNavMesh);
		m_pNavMesh = 0;
		return false;		
	}

	if (dtStatusFailed(m_pNavQuery->init(m_pNavMesh, 2048)))
	{
		LOG_ERROR("Error while building navigation mesh: unable to init detour query");
		dtFreeNavMesh(m_pNavMesh);
		m_pNavMesh = 0;
		return false;
	}

	return true;
}

std::shared_ptr<Graphics2::Mesh> NavigationScene::CreateGraphicsMesh() const
{
	if (!m_pMeshDetail)
	{
		return nullptr;
	}

	//int numVerts = input->GetNumVerts();
	//int numTris = input->GetNumTris();
	int numVerts = m_pMeshDetail->nverts;
	int numTris = m_pMeshDetail->ntris;
	Vertex* pVerts = new Vertex[numVerts];
	unsigned short* pTris = new unsigned short[numTris * 3];
	
	/*
	for (int i = 0; i < numVerts; ++i)
	{
		pVerts[i] = Vertex(input->GetVertices()[i], Vector3::ZERO, Vector2(0.f, 0.f));
	}
	for (int i = 0; i < numTris; ++i)
	{
		pTris[i * 3 + 0] = input->GetIndices()[i * 3 + 0];
		pTris[i * 3 + 1] = input->GetIndices()[i * 3 + 1];
		pTris[i * 3 + 2] = input->GetIndices()[i * 3 + 2];
	}*/

	int currentVert = 0;
	int currentTri = 0;

	for (int i = 0; i < m_pMeshDetail->nmeshes; ++i)
	{
		const unsigned int* m = &m_pMeshDetail->meshes[i * 4];
		const unsigned int bverts = m[0];
		const int nverts = (int)m[1];
		const unsigned int btris = m[2];
		const int ntris = (int)m[3];
		const float* verts = &m_pMeshDetail->verts[bverts * 3];
		const unsigned char* tris = &m_pMeshDetail->tris[btris * 4];

		const int baseVert = currentVert;
		for (int j = 0; j < nverts; ++j)
		{
			pVerts[currentVert++] = Vertex(Vector3(verts[j * 3 + 0], verts[j * 3 + 1], verts[j * 3 + 2]), Vector3::ZERO, Vector2(0.f, 0.f));
		}
		for (int j = 0; j < ntris; ++j)
		{
			const int triBase = currentTri++ * 3;
			//Change the winding order of the triangle
			pTris[triBase + 0] = baseVert + tris[j * 4 + 0];
			pTris[triBase + 1] = baseVert + tris[j * 4 + 1];
			pTris[triBase + 2] = baseVert + tris[j * 4 + 2];
		}
	}

	numVerts = currentVert;
	numTris = currentTri;

	Material mat;
	mat.m_type = Graphics2::MT_WALKMESH;

	std::shared_ptr<Graphics2::Mesh> pMesh(CrNew Graphics2::Mesh(Graphics2::gpDevice));
	//pMesh->setMaterial(mat);
	pMesh->setVertexData(pVerts, numVerts);
	pMesh->setIndexData(pTris, numTris * 3);

	delete [] pVerts;
	delete [] pTris;

	return pMesh;
}