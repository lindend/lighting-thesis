#pragma once

#include "CrazeMath.h"
#include "Intersection/BoundingBox.h"

#include "Memory/MemoryManager.h"

namespace Craze
{
	class NavInputMesh
	{
		CRAZE_POOL_ALLOC(NavInputMesh);
	public:
		NavInputMesh() : m_pVertices(nullptr), m_pIndices(nullptr), m_NumVerts(0), m_NumTris(0), m_pBB(nullptr) {}
		~NavInputMesh() { Free(); }

		void Set(const Vec3* pVerts, int vtxStride, int numVerts, const unsigned short* pIndices, int numTris);
		void Set(const Vec3* pVerts, int vtxStride, int numVerts, const int* pIndices, int numTris);

		const int GetNumVerts() const { return m_NumVerts; }
		const int GetNumTris() const { return m_NumTris; }

		const Vec3* GetVertices() const { return m_pVertices; }
		const int* GetIndices() const { return m_pIndices; }

		const BoundingBox* GetBB() const { return m_pBB; }
	private:
		void SetVerts(const Vec3* pVerts, int vtxStride, int numVerts);
		void Free();

		Vec3* m_pVertices;
		int* m_pIndices;
		int m_NumVerts;
		int m_NumTris;
		BoundingBox* m_pBB;

		NavInputMesh(const NavInputMesh &);
		NavInputMesh & operator=(const NavInputMesh &);
	};
}