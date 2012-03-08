#include "CrazeGraphicsPCH.h"
#include "SkyVisibility.h"

#include "../Graphics.h"
#include "../Device.h"
#include "../Texture/RenderTarget.h"
#include "../Buffer/Buffer.h"
#include "../Mesh.h"
#include "../Effect/PosMapEffect.h"

using namespace Craze;
using namespace Craze::Graphics2;

bool Craze::Graphics2::CreatePositionMap(std::shared_ptr<Mesh> pMesh, std::shared_ptr<RenderTarget> pTargetPos, std::shared_ptr<RenderTarget> pTargetNormal)
{
	std::shared_ptr<RenderTarget> pRTs[] = { pTargetPos, pTargetNormal };
	gpDevice->SetRenderTargets(pRTs, 2, nullptr);
	gFxPosMap.set();
	pMesh->draw();
	gpDevice->SetRenderTarget(nullptr);

	return true;
}

bool Craze::Graphics2::CalculateSkyVisibility(std::shared_ptr<SRVBuffer> pTris, std::shared_ptr<SRVBuffer> pKdTree, std::shared_ptr<Texture> pPosMap, 
	std::shared_ptr<Texture> pNormalMap, std::shared_ptr<RenderTarget> pOcclMap)
{
	static const ComputeShaderResource* res = EffectHelper::LoadShaderFromResource<ComputeShaderResource>("SkyOcclusion.csh");
	if (!res || !res->m_shader)
	{
		LOG_ERROR("Unable to compile sky visibility shader");
		return false;
	}

	auto pUAV = pOcclMap->GetUAV();
	unsigned int defCount = -1;
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &pUAV, &defCount);

	ID3D11ShaderResourceView* pSRVs[] = { pTris->GetSRV(), pKdTree->GetSRV(), pPosMap->GetResourceView(), pNormalMap->GetResourceView() };
	gpDevice->GetDeviceContext()->CSSetShaderResources(0, 4, pSRVs);
	gpDevice->GetDeviceContext()->CSSetShader(res->m_shader, NULL, NULL);
	
	gpDevice->GetDeviceContext()->Dispatch(pPosMap->GetWidth() / 16, pPosMap->GetHeight() / 16, 1);

	pUAV = nullptr;
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &pUAV, &defCount);

	ZeroMemory(pSRVs, sizeof(void*) * 4);
	gpDevice->GetDeviceContext()->CSSetShaderResources(0, 4, pSRVs);

	return true;
}