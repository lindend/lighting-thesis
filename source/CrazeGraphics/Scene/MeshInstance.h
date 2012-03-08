#pragma once

#include "Intersection/BoundingBox.h"

#include "../DrawList.h"
#include "../Mesh.h"

namespace Craze
{
	namespace Graphics2
	{
		class Scene;
		class MeshInstance
		{
		public:
			MeshInstance(std::shared_ptr<Mesh> pMesh, Scene* pScene)
			{
				m_RI.m_mesh = pMesh.get();
				m_SortId.type = RT_TERRAIN;

				if (m_Material.m_decal)
				{
					m_SortId.textureId = m_Material.m_decal->GetID();
				}

				m_SortId.materialType = m_Material.m_type;
				m_SortId.viewport = 0;

				//m_pBoundingBox = pMesh->getBoundingBox();
			}
			~MeshInstance() {}

			void SetTransform(const Matrix4& transform);
			void SetMesh(std::shared_ptr<Mesh> pMesh, Material material);// { m_RI.m_mesh = pMesh.get(); m_RI.m_material = material; }

			void Draw(DrawList* pDL, float depth);
			void Remove();
	
			RenderItem m_RI;
			SortingId m_SortId;

			BoundingBox* m_pBoundingBox;

		private:
			Scene* m_pScene;
			Material m_Material;

		public:
			bool visible : 1;
			bool dynamic : 1;
			bool isLight : 1;
			bool isFixated : 1; 

		private:
			MeshInstance(const MeshInstance&);
			MeshInstance operator=(const MeshInstance&);
			
		};
	}
}