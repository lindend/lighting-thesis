#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "Geometry/TerrainChunk.h"
#include "Texture/DynamicTexture.h"
#include "Scene/Scene.h"

#include "Intersection/Ray.h"


namespace Craze
{
	class CRAZEENGINE_EXP Terrain
	{
	public:
		static std::shared_ptr<Terrain> CreateFromFile(Graphics2::Device* pDevice, std::string fileName);

		optional<Vector3> GetRayIntersectionPoint(const Ray& ray);

		optional<float> GetHeight(float xPos, float zPos);

		void AddToScene(Graphics2::Scene* pScene);

	protected:

		Terrain() {}

		unsigned int GetArrayIndex(unsigned int x, unsigned int y) const;

		unsigned int m_WidthSegments;
		unsigned int m_HeightSegments;
		float m_SegmentSize;
		unsigned int m_VertsPerSegment;
		unsigned int m_EffectiveVertWidth;
		unsigned int m_EffectiveVertHeight;
        float* m_pTerrainHeightData;
		Vector4* m_pTerrainTextureData0;
		Graphics2::Device* m_pDevice;

		std::shared_ptr<Graphics2::TextureArray> m_pDecals;

		std::vector<Graphics2::TerrainChunk> m_pTerrainChunks;

		std::vector<std::shared_ptr<Graphics2::Texture>> m_HeightMapTextures;
		std::vector<Vector2> m_HeightMapExtremes;
	};

	enum TERRAIN_OPERATION
	{
		TERRAIN_OPERATION_PAINT,
		TERRAIN_OPERATION_RAISE,
	};

	class CRAZEENGINE_EXP EditableTerrain : public Terrain
	{
	public:
		static std::shared_ptr<EditableTerrain> CreateNewEditable(Graphics2::Device* pDevice, unsigned int widthSegments, unsigned int heightSegments, float segmentSize, unsigned int vertsPerSegmentSide
			, unsigned int decalWidth, unsigned int decalHeight);
		static std::shared_ptr<EditableTerrain> CreateEditableFromFile(Graphics2::Device* pDevice, std::string fileName);

		void SaveToFile(std::string fileName);

		void Edit(TERRAIN_OPERATION operation, const Vector2& aroundPoint, float amount, float radius, float softness, unsigned int layer);

		void SetLayerTexture(unsigned int layer, std::string decal, std::string slope, std::string decalBump, std::string slopeBump);

	private:

		void UpdateHeightSegment(unsigned int x, unsigned int y);
		void UpdateTextureSegment(unsigned int x, unsigned int y);

		std::vector<std::shared_ptr<Graphics2::DynamicTexture>> m_EditHeightTextures;
		std::vector<std::shared_ptr<Graphics2::DynamicTexture>> m_EditTexturingTextures;
	};
}
