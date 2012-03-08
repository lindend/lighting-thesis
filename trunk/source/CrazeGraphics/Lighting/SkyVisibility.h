#pragma once
#include <memory>

namespace Craze
{
	namespace Graphics2
	{
		class Mesh;
		class Texture;
		class SRVBuffer;
		class RenderTarget;

		bool CreatePositionMap(std::shared_ptr<Mesh> pMesh, std::shared_ptr<RenderTarget> pTargetPos, std::shared_ptr<RenderTarget> pTargetNormal);
		bool CalculateSkyVisibility(std::shared_ptr<SRVBuffer> pTris, std::shared_ptr<SRVBuffer> pKdTree, std::shared_ptr<Texture> pPosMap, std::shared_ptr<Texture> pNormalMap, std::shared_ptr<RenderTarget> pOutOcclMap);
	}
}