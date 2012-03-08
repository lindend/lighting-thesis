#include "CrazeEngine.h"
#include "Terrain.h"
#include "Texture/TextureArray.h"
#include "Device.h"

#include "Intersection/Intersection.h"

#include "Util/MersenneTwister.h"
#include "Graphics.h"
#include "Scene/ModelNode.h"

using namespace Craze;
using namespace Craze::Graphics2;


void Terrain::AddToScene(Scene* pScene)
{
	float startPosX = 0.0f;//-m_SegmentSize * m_WidthSegments / 2.0f;
	float startPosY = 0.0f;//-m_SegmentSize * m_HeightSegments / 2.0f;

	for (unsigned int i = 0; i < m_HeightSegments; ++i)
	{
		for (unsigned int j = 0; j < m_WidthSegments; ++j)
		{
			//MeshInstance* pInst = pScene->AddTerrain(&m_pTerrainChunks[i * m_WidthSegments + j], NODE_DISPLAYABLE);
			//pInst->SetTransform(Matrix4::CreateTranslation(Vector3(startPosX + j * m_SegmentSize, 0.0f, startPosY + i * m_SegmentSize)));
		}
	}
}

unsigned int Terrain::GetArrayIndex(unsigned int x, unsigned int y) const
{
	return x + m_EffectiveVertHeight * y;

	/*unsigned int vps = m_VertsPerSegment / 10;
	unsigned int segmentSize = vps * vps;
	unsigned int segmentNum = x / vps + y / vps * m_WidthSegments * 10;

	return x % vps + (y % vps) * vps + segmentNum * segmentSize;
	*/
	//return x + y * m_EffectiveVertWidth;

}

optional<float> Terrain::GetHeight(float xPos, float zPos)
{
	float vertPosX = (xPos/* + m_SegmentSize * m_WidthSegments / 2.0f*/) / (m_WidthSegments * m_SegmentSize) * m_EffectiveVertWidth;
	float vertPosY = (zPos/* + m_SegmentSize * m_HeightSegments / 2.0f*/) / (m_HeightSegments * m_SegmentSize) * m_EffectiveVertHeight;

	if (vertPosX < 0 || vertPosY < 0 || vertPosX + 1 >= m_EffectiveVertWidth || vertPosY + 1 >= m_EffectiveVertHeight)
	{
		return optional<float>();
	}

	float height0 = m_pTerrainHeightData[GetArrayIndex((unsigned int)vertPosX,		(unsigned int)vertPosY)];
	float height1 = m_pTerrainHeightData[GetArrayIndex((unsigned int)vertPosX + 1,	(unsigned int)vertPosY)];
	float height2 = m_pTerrainHeightData[GetArrayIndex((unsigned int)vertPosX,		(unsigned int)vertPosY + 1)];
	float height3 = m_pTerrainHeightData[GetArrayIndex((unsigned int)vertPosX + 1,	(unsigned int)vertPosY + 1)];

	return optional<float>(Lerp(Lerp(height0, height2, vertPosX - (int)vertPosX), Lerp(height1, height3, vertPosX - (int)vertPosX), vertPosY - (int)vertPosY));
}



optional<Vector3> Terrain::GetRayIntersectionPoint(const Ray& ray)
{
	float vertexDistance = m_SegmentSize / ((float)m_VertsPerSegment - 1.0f);
	float tPeriodX = Abs(vertexDistance / ray.m_Dir->x);
	float tPeriodY = Abs(vertexDistance / ray.m_Dir->z);

	Vector2 vertSpacePos;
	vertSpacePos.x = ray.m_Origin->x / (m_WidthSegments * m_SegmentSize) * m_EffectiveVertWidth;
	vertSpacePos.y = ray.m_Origin->z / (m_WidthSegments * m_SegmentSize) * m_EffectiveVertWidth;

	float t = 0.0f;

	bool insideX = vertSpacePos.x >= 0.f && vertSpacePos.x < m_EffectiveVertWidth;
	bool insideY = vertSpacePos.y >= 0.f && vertSpacePos.y < m_EffectiveVertHeight;

	Vector3 origin = ray.m_Origin;

	if (!insideX || !insideY)
    {
        __declspec(align(16)) Plane planes[] = {
			Plane(Vector3::ZERO, Vector3::RIGHT),
			Plane(Vector3(0.0f, 0.0f, m_EffectiveVertHeight * vertexDistance), Vector3::RIGHT),
			Plane(Vector3::ZERO, Vector3::FORWARD),
			Plane(Vector3(m_EffectiveVertWidth * vertexDistance, 0.0f, 0.0f), Vector3::FORWARD)
			};

        RayTestResult res = Intersection::Test(ray, planes);

		if (insideY)
		{
			t = res.results[1] < res.results[0] && res.results[1] > 0.f ? res.results[1] : res.results[0];
		} else if(insideX)
		{
			t = res.results[3] < res.results[2] && res.results[3] > 0.f ? res.results[3] : res.results[2];
		} else
		{
			float xmin = res.results[1] < res.results[0] && res.results[1] > 0.f ? res.results[1] : res.results[0];
			float ymin = res.results[3] < res.results[2] && res.results[3] > 0.f ? res.results[3] : res.results[2];

			t = Max(xmin, ymin);
		}

		origin = ray.m_Origin + ray.m_Dir * t;
		vertSpacePos.x = (ray.m_Origin->x + ray.m_Dir->x * t) / (m_WidthSegments * m_SegmentSize) * m_EffectiveVertWidth;
		vertSpacePos.y = (ray.m_Origin->z + ray.m_Dir->y * t) / (m_WidthSegments * m_SegmentSize) * m_EffectiveVertWidth;
    }

	


	float tNextX = ray.m_Dir->x >= 0 ? 1.0f - Frac(vertSpacePos.x) : Frac(vertSpacePos.x);
	tNextX /= abs(ray.m_Dir->x);

	float tNextY = ray.m_Dir->z >= 0 ? 1.0f - Frac(vertSpacePos.y) : Frac(vertSpacePos.y);
	tNextY /= abs(ray.m_Dir->z);

	
	float tPrev = 0.0f;
	float prevHeight = GetHeight(origin->x, origin->z).Get();
	bool startsOver = origin->y > prevHeight;

	Vector3 relpos = ray.m_Origin + ray.m_Dir * t;
	while (relpos->x >= 0 && relpos->z >= 0 && relpos->x < m_EffectiveVertWidth * vertexDistance && relpos->z < m_EffectiveVertHeight * vertexDistance)
	{
		if (tNextX <= tNextY)
		{
			t += tNextX;
			tNextY -= tNextX;
			tNextX = tPeriodX;
		} else
		{
			t += tNextY;
			tNextX -= tNextY;
			tNextY = tPeriodY;
		}

		Vector3 pos = ray.m_Origin + ray.m_Dir * t;
		optional<float> tstHeight = GetHeight(pos->x, pos->z);

		if (!tstHeight.Valid())
		{
			return optional<Vector3>();
		}

		float height = tstHeight.Get();
		bool over = pos->y > height;

		if (over != startsOver)
		{
			float dH = height - prevHeight;
			float drH = pos->y - (ray.m_Origin + ray.m_Dir * tPrev)->y;
			float tP = ((ray.m_Origin + ray.m_Dir * tPrev)->y - prevHeight) / (dH - drH);

			float tRes = tPrev + tP * (t - tPrev);

			return optional<Vector3>(Vector3(ray.m_Origin + ray.m_Dir * tRes));
		}

		relpos = ray.m_Origin + ray.m_Dir * t;
		tPrev = t;
		prevHeight = height;
	}

	return optional<Vector3>();
}

void EditableTerrain::Edit(TERRAIN_OPERATION operation, const Vector2& aroundPoint, float amount, float radius, float softness, unsigned int layer)
{

	float vertexDistance = m_SegmentSize / (m_VertsPerSegment - 1);

	Vector2 relativePos = aroundPoint;// - Vector2(-m_SegmentSize * m_WidthSegments / 2.0f, -m_SegmentSize * m_HeightSegments / 2.0f);

	Vector2 vertSpacePos = relativePos * (1.0f / vertexDistance);

	unsigned int startX = Max<int>(0, (int)(vertSpacePos.x - radius / vertexDistance - 1));
	unsigned int startY = Max<int>(0, (int)(vertSpacePos.y - radius / vertexDistance - 1));

	unsigned int num = (unsigned int)(2 * (radius / vertexDistance + 1));

	float sqRadius = radius * radius;

	for (unsigned int y = startY; y < startY + num && y < m_EffectiveVertWidth; ++y)
	{
		for (unsigned int x = startX; x < startX + num && x < m_EffectiveVertWidth; ++x)
		{
			float distance = (Vector2(x * vertexDistance, y * vertexDistance) - relativePos).GetSquareLen();

			if (distance < sqRadius)
			{
				float distFactor = 1.0f - Sqrt(distance) / radius;

				float scaledAmount = Lerp(0, amount, distFactor);

				if (operation == TERRAIN_OPERATION_RAISE)
				{
					m_pTerrainHeightData[GetArrayIndex(x, y)] += scaledAmount;
				} else if(operation == TERRAIN_OPERATION_PAINT)
				{
					float* pData = (float*)&m_pTerrainTextureData0[GetArrayIndex(x, y)];
					pData[0] *= 1.0f - scaledAmount;
					pData[1] *= 1.0f - scaledAmount;
					pData[2] *= 1.0f - scaledAmount;
					pData[3] *= 1.0f - scaledAmount;
					*((float*)pData + layer) += scaledAmount;
				}
			}
		}
	}

	int startSegmentX = startX / (m_VertsPerSegment - 1);
	int startSegmentY = startY / (m_VertsPerSegment - 1);

	startSegmentX = Max(0, startSegmentX);

	startSegmentY = Max(0, startSegmentY);

	int numSegments = num / (m_VertsPerSegment - 1) + 2;

	for (unsigned int y = (unsigned int)startSegmentY; y < (unsigned int)(startSegmentY + numSegments) && y < m_HeightSegments; ++y)
	{
		for (unsigned int x = (unsigned int)startSegmentX; x < (unsigned int)(startSegmentX + numSegments) && x < m_WidthSegments; ++x)
		{
			if (operation == TERRAIN_OPERATION_RAISE)
			{
				UpdateHeightSegment(x, y);
			} else if(operation == TERRAIN_OPERATION_PAINT)
			{
				UpdateTextureSegment(x, y);
			}
		}
	}
}

void EditableTerrain::UpdateHeightSegment(unsigned int x, unsigned int y)
{
	TextureLock* pLock = m_EditHeightTextures[x + y * m_WidthSegments]->Lock();

	unsigned int firstVertX = (m_VertsPerSegment - 1) * x;
	unsigned int firstVertY = (m_VertsPerSegment - 1) * y;

    float vertDist = m_SegmentSize / (m_VertsPerSegment - 1);
    float dblDist = vertDist + vertDist;

	for (unsigned int i = 0; i < m_VertsPerSegment; ++i)
	{
	    float dy = i == 0 || i == m_VertsPerSegment - 1 ? vertDist : dblDist;

		for (unsigned int j = 0; j < m_VertsPerSegment; ++j)
		{
		    float dx = j == 0 || j == m_VertsPerSegment - 1 ? vertDist : dblDist;

			float height = m_pTerrainHeightData[GetArrayIndex(firstVertX + j, firstVertY + i)];

			float lHeight = j > 0 ?		m_pTerrainHeightData[GetArrayIndex(firstVertX + j - 1, firstVertY + i)] : height;
			float rHeight = m_VertsPerSegment - j - 1 > 0 ?		m_pTerrainHeightData[GetArrayIndex(firstVertX + j + 1, firstVertY + i)] : height;

			Vector3 dirx = Vector3::Normalize(Vector3(dx, rHeight - lHeight, 0.0f));

			float uHeight = i > 0 ?		m_pTerrainHeightData[GetArrayIndex(firstVertX + j, firstVertY + i - 1)] : height;
			float dHeight = m_VertsPerSegment - i - 1 > 0 ?		m_pTerrainHeightData[GetArrayIndex(firstVertX + j, firstVertY + i + 1)] : height;

			Vector3 diry = Vector3::Normalize(Vector3(0.0f, uHeight - dHeight, dy));

			*pLock->GetPixel<Vector4>(j, i) = Vector4(diry.Cross(dirx), height);
		}
	}

	pLock->Unlock();

}

void EditableTerrain::UpdateTextureSegment(unsigned int x, unsigned int y)
{
	TextureLock* pLock = m_EditTexturingTextures[x + y * m_WidthSegments]->Lock();

	unsigned int firstVertX = (m_VertsPerSegment - 1) * x;
	unsigned int firstVertY = (m_VertsPerSegment - 1) * y;

	for (unsigned int i = 0; i < m_VertsPerSegment; ++i)
	{
		for (unsigned int j = 0; j < m_VertsPerSegment; ++j)
		{
			*pLock->GetPixel<Vector4>(j, i) = m_pTerrainTextureData0[GetArrayIndex(firstVertX + j, firstVertY + i)];
		}
	}

	pLock->Unlock();
}

std::shared_ptr<EditableTerrain> EditableTerrain::CreateNewEditable(Graphics2::Device* pDevice, unsigned int widthSegments, unsigned int heightSegments, float segmentSize, unsigned int vertsPerSegmentSide
	,unsigned int decalWidth, unsigned int decalHeight)
{
	std::shared_ptr<EditableTerrain> pResult(new EditableTerrain());

	pResult->m_WidthSegments = widthSegments;
	pResult->m_HeightSegments = heightSegments;
	pResult->m_SegmentSize = segmentSize;
	pResult->m_VertsPerSegment = vertsPerSegmentSide;
	pResult->m_pDevice = pDevice;

	pResult->m_EditHeightTextures.reserve(widthSegments * heightSegments);
	pResult->m_HeightMapExtremes.reserve(widthSegments * heightSegments);
	pResult->m_pTerrainChunks.reserve(widthSegments * heightSegments);

	pResult->m_EffectiveVertHeight = (vertsPerSegmentSide - 1) * (heightSegments - 1) + vertsPerSegmentSide;
	pResult->m_EffectiveVertWidth = (vertsPerSegmentSide - 1) * (widthSegments - 1) + vertsPerSegmentSide;

	pResult->m_pDecals = TextureArray::Create(pDevice, decalWidth, decalHeight, 8, 0);

	//pResult->m_pDecals->LoadTextureIntoSlice(0, Texture::CreateFromFile(pDevice, "Media/ground_decal.png"));
	//pResult->m_pDecals->LoadTextureIntoSlice(4, Texture::CreateFromFile(pDevice, "Media/rock_decal.png"));

	unsigned int numPoints = pResult->m_EffectiveVertHeight * pResult->m_EffectiveVertWidth;
	pResult->m_pTerrainHeightData = new float[numPoints];
	pResult->m_pTerrainTextureData0 = new Vector4[numPoints];

	for (unsigned int i = 0; i < numPoints; ++i)
	{
		pResult->m_pTerrainTextureData0[i] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	}

	ZeroMemory(pResult->m_pTerrainHeightData, sizeof(float) * numPoints);

/*	std::shared_ptr<Texture> pBump = Texture::CreateFromFile(pDevice, "Media/rock_bump.png");

	for (unsigned int i = 0; i < heightSegments; ++i)
	{
		for (unsigned int j = 0; j < widthSegments; ++j)
		{
			pResult->m_EditHeightTextures.push_back(DynamicTexture::Create(pDevice, vertsPerSegmentSide, vertsPerSegmentSide, TEXTURE_FORMAT_VECTOR4, 2));
			pResult->m_EditTexturingTextures.push_back(DynamicTexture::Create(pDevice, vertsPerSegmentSide, vertsPerSegmentSide, TEXTURE_FORMAT_VECTOR4, 2));
			pResult->m_HeightMapExtremes.push_back(Vector2(0.0f, 0.0f));

			TextureLock* pLock = pResult->m_EditHeightTextures.back()->Lock();
			TextureLock* pLockTex = pResult->m_EditTexturingTextures.back()->Lock();

			for (unsigned int y = 0; y < vertsPerSegmentSide; ++y)
			{
				for (unsigned int x = 0; x < vertsPerSegmentSide; ++x)
				{

					*pLock->GetPixel<Vector4>(x, y) = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
					*pLockTex->GetPixel<Vector4>(x, y) = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
				}
			}

			pLock->Unlock();
			pLockTex->Unlock();

			pResult->m_pTerrainChunks.push_back(TerrainChunk(pDevice, NULL, segmentSize, vertsPerSegmentSide, pResult->m_EditHeightTextures.back(), pResult->m_pDecals, pBump,
				pResult->m_EditTexturingTextures.back()));
		}
	}*/

	return pResult;
}

void EditableTerrain::SetLayerTexture(unsigned int layer, std::string decal, std::string slope, std::string decalBump, std::string slopeBump)
{
	assert("Invalid layer, must be >= 0 and <= 3" && layer >= 0 && layer <= 3);
	//m_pDecals->LoadTextureIntoSlice(layer, gGfxResources.LoadTexture(decal)->pItem);
	//m_pDecals->LoadTextureIntoSlice(layer + 4, gGfxResources.LoadTexture(slope)->pItem);
}

std::shared_ptr<EditableTerrain> EditableTerrain::CreateEditableFromFile(Graphics2::Device* pDevice, std::string fileName)
{
	return std::shared_ptr<EditableTerrain>(nullptr);
}

void EditableTerrain::SaveToFile(std::string fileName)
{
	FILE* f;
	fopen_s(&f, fileName.c_str(), "w+b");

	fwrite(&m_WidthSegments, sizeof(unsigned int), 1, f);
	fwrite(&m_HeightSegments, sizeof(unsigned int), 1, f);

	fwrite(&m_VertsPerSegment, sizeof(unsigned int), 1, f);
	fwrite(&m_SegmentSize, sizeof(float), 1, f);


	int strLength = 0;
	for(int i = 0; i < 4; ++i)
	{
		//strLength = fileNames[i].length() + 1;
		//fwrite(&strLength, sizeof(unsigned int), 1, f);
		//fwrite(fileNames[i].c_str(), sizeof(char), strLength, f);
	}

	fwrite(m_pTerrainHeightData, sizeof(float), m_EffectiveVertHeight * m_EffectiveVertWidth, f);
	fwrite(m_pTerrainTextureData0, sizeof(Vector4), m_EffectiveVertHeight * m_EffectiveVertWidth, f);

	fclose(f);
}
