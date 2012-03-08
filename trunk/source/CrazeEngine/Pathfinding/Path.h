#pragma once
#include "CrazeMath.h"
#include "Memory/MemoryManager.h"

namespace Craze
{
	class Path
	{
		CRAZE_ALLOC();
	public:
		Path(int maxVerts = 512) : m_MaxVerts(maxVerts), m_NumVerts(0) { m_pVerts = new Vec3[maxVerts]; m_pPathFlags = new unsigned char[maxVerts]; }
		~Path() { delete [] m_pVerts; delete [] m_pPathFlags; }

		Vec3* GetVerts() { return m_pVerts; }
		unsigned char* GetPathFlags() { return m_pPathFlags; }
		void SetNumVerts(int numVerts) { m_NumVerts = numVerts; }
		int GetMaxVerts() const { return m_MaxVerts; }
		int GetNumVerts() const { return m_NumVerts; }

	private:
		Path(const Path&);
		Path& operator=(const Path&);

		Vec3 startPoint;
		Vec3 endPoint;
		Vec3* m_pVerts;
		unsigned char* m_pPathFlags;
		int m_NumVerts;
		int m_MaxVerts;
	};
}