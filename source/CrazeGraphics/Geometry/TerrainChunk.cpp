#include "CrazeGraphicsPCH.h"
#include "TerrainChunk.h"
#include "Mesh.h"
#include "../Texture/TextureArray.h"
#include "Device.h"
#include "Graphics.h"

using namespace Craze;
using namespace Craze::Graphics2;

TerrainChunk::TerrainChunk(Device* pDevice, Camera* pCamera, float size, unsigned int vps, std::shared_ptr<Texture> pHeightMap, std::shared_ptr<TextureArray> pDecals,
	std::shared_ptr<Texture> pBumpMaps, std::shared_ptr<Texture> pTextureWeights0)
{
	m_pDevice = pDevice;
	m_pCamera = pCamera;

	m_Material.m_Specular = 10;
	m_Material.m_Type = MT_TERRAIN;
	m_Material.m_Decal = pDecals;
	m_Material.m_Bump = pBumpMaps;

	m_pHeightMap = pHeightMap;//Texture::CreateFromFile(pDevice, "Media/heightmap1_working3.png");
	m_pTextureWeights0 = pTextureWeights0;

	m_Material.m_pSpecialTextures.reset(new TexturePtr[2]);
	m_Material.m_pSpecialTextures[0] = m_pHeightMap;
	m_Material.m_pSpecialTextures[1] = m_pTextureWeights0;

	m_pTerrainMesh.reset(CrNew Mesh(pDevice));

	Vertex* verts = new Vertex[vps * vps];
	unsigned short* indices = new unsigned short[(vps - 1) * (vps - 1) * 6];

	float uvStart = (1.0f / (float)vps) * 0.5f;
	float uvEnd = 1.0f - uvStart;

	for(unsigned int i = 0; i < vps; ++i)
	{
		float xpos = (float)i * (size / (vps - 1));// - (size / 2.0f);
		for (unsigned int j = 0; j < vps; ++j)
		{
			Vector2 uv = Vector2(Lerp(uvStart, uvEnd, (float)i / (vps - 1)), Lerp(uvStart, uvEnd, (float)j / (vps - 1)));
			float zpos = (float)j * (size / (vps - 1));// - (size / 2.0f);
			verts[i * vps + j] = Vertex(Vector3(xpos, 0.0f, zpos), Vector3::UP, uv);
		}
	}

	for (unsigned int i = 0; i < vps - 1; ++i)
	{
		for (unsigned int j = 0; j < vps - 1; ++j)
		{
			indices[(i * (vps - 1) + j) * 6 + 0] = i * vps + j;
			indices[(i * (vps - 1) + j) * 6 + 1] = (i + 1) * vps + (j + 1);
			indices[(i * (vps - 1) + j) * 6 + 2] = (i + 1) * vps + j;
			
			indices[(i * (vps - 1) + j) * 6 + 3] = i * vps + j;
			indices[(i * (vps - 1) + j) * 6 + 4] = i * vps + (j + 1);
			indices[(i * (vps - 1) + j) * 6 + 5] = (i + 1) * vps + (j + 1);
		}
	}

	m_pTerrainMesh->SetVertexData(verts, vps * vps);
	m_pTerrainMesh->SetIndexData(indices, (vps - 1) * (vps - 1) * 6);
}