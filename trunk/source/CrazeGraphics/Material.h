#pragma once

#include "Memory/MemoryManager.h"

#include "Texture/Texture.h"

namespace Craze
{
	namespace Graphics2
	{
		enum MATERIAL_TYPE : unsigned char
		{
			MT_NORMAL,
			MT_TERRAIN,
			MT_WATER,
			MT_FOLIAGE,
			MT_WALKMESH,
		};

		class Material
		{
			CRAZE_POOL_ALLOC(Material);
		public:
			Material() : m_specular(0.f), m_lightMapIndex(0), m_color(0xFFFFFFFF), m_isTransparent(false), m_type(MT_NORMAL) { }
			~Material() { }

			float m_specular;
            unsigned int m_lightMapIndex;
			u32 m_color;
			bool m_isTransparent;
			TexturePtr m_decal;
			TexturePtr m_bump;
			TexturePtr m_lightMap;
			TexturePtr m_skyVisMap;

			MATERIAL_TYPE m_type;
		};
	}
}
