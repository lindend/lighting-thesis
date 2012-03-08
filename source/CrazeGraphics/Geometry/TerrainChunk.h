#pragma once

#include "../Material.h"

namespace Craze
{
	namespace Graphics2
	{
		class Mesh;
		class Camera;
		class Texture;
		class TextureArray;
		class Device;

		class TerrainChunk
		{
			CRAZE_ALLOC();
		public:
			TerrainChunk(Device* pDevice, Camera* pCamera, float size, unsigned int numVertsPerSide, std::shared_ptr<Texture> pHeightMap, std::shared_ptr<TextureArray> pDecals,
				std::shared_ptr<Texture> pBumpMap, std::shared_ptr<Texture> pTextureWeights0);

			std::shared_ptr<Mesh> m_pTerrainMesh;
			std::shared_ptr<Texture> m_pHeightMap;
			std::shared_ptr<Texture> m_pTextureWeights0;
			std::shared_ptr<TextureArray> m_pDecals;

			Material m_Material;

			Device* m_pDevice;
			Camera* m_pCamera;
		};
	}
}