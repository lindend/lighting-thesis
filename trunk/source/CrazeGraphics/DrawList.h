#pragma once
#include <memory>
#include <vector>
#include <algorithm>

#include "MathTypes.h"
#include "Material.h"
#include "Mesh.h"

namespace Craze
{
	namespace Graphics2
	{
		class Mesh;
		enum RENDERTYPE
		{
			RT_MESH,
			RT_TERRAIN,
			RT_SKINNED,
		};

		struct SortingId
		{
			unsigned depth : 24;
			unsigned textureId : 20;
			unsigned materialType : 4;
			//Type of the render item: Terrain, Mesh, Skinned mesh etc...
			unsigned type : 3;
			unsigned transparency : 1;
			//If several viewports (for example mirrors etc.) are used...
			unsigned viewport : 3;

			static u64 encode(u32 depth, u32 textureId, u32 materialType, u32 type, bool transparency, u32 viewport)
			{
				return
					(u64)depth << 0 |
					(u64)textureId << 24 |
					(u64)materialType << 44 |
					(u64)type << 48 |
					(u64)transparency << 51 |
					(u64)viewport << 52; 
			}

			u64 encode() const
			{
#if _MSC_VER == 1600
				return *(u64*)this;
#else
				return encode(depth, textureId, materialType, type, transparency, viewport);
#endif
			}

			operator u64() const { return encode(); }
		};

		class RenderItem
		{
		public:
			const Matrix4* m_transform;
			const Material* m_material;
			const Mesh* m_mesh;
		};


		class DrawList
		{
		public:
			const std::pair<u64, RenderItem>* begin() const { return m_items.empty() ? nullptr : &m_items.front(); }
			const std::pair<u64, RenderItem>* end() const { return m_items.empty() ? nullptr : &m_items.back(); }

			void add(RENDERTYPE type, u32 depth, const Mesh* mesh, const Material* material, const Matrix4* transform)
			{
				u64 sortId = SortingId::encode(depth, material->m_decal->GetID(), material->m_type, type, material->m_isTransparent, 0);
				RenderItem itm = {transform, material, mesh};
				m_items.push_back(std::make_pair(21, itm));
			}

			void clear() { m_items.clear(); }
			void sort() { std::sort(m_items.begin(), m_items.end(), sortFunct); }

		private:
			std::vector<std::pair<u64, RenderItem>> m_items;

			static bool sortFunct(const std::pair<u64, RenderItem>& v0, const std::pair<u64, RenderItem>& v1)
			{
				return v0.first > v1.first;
			}
		};

		
	}
}