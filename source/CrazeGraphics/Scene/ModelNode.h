#pragma once
#include <vector>

#include "Memory/MemoryManager.h"

#include "MathTypes.h"
#include "CrazeMath.h"
#include "Intersection/BoundingBox.h"

namespace Craze
{
	class Matrix4;

	namespace Graphics2
	{
		class Mesh;
		class DrawList;

		class ModelNode
		{
			CRAZE_ALLOC_ALIGN(16);

			friend class Scene;
		public:
			ModelNode(u32 firstMesh, u32 numMeshes) : m_firstMesh(firstMesh), m_numMeshes(numMeshes), m_dirty(true) {}

			u32 getFirstMesh() const { return m_firstMesh; }
			u32 getNumMeshes() const { return m_numMeshes; }

			void setTransform(const Matrix4& tfm) { m_transform = tfm; m_dirty = true; }
			const Matrix4& getTransform() const { return m_transform; }

			const BoundingBox& getBoundingBox() const { return m_bbox; }

			bool isDirty() const { return m_dirty; }
			void unflagDirty() { m_dirty = false; }
		private:
			Matrix4 m_transform;
			BoundingBox m_bbox;
			u32 m_firstMesh;
			u32 m_numMeshes;
			bool m_dirty;
		};
	}
}
