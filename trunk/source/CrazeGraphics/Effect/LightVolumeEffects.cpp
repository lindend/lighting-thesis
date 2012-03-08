#include "CrazeGraphicsPCH.h"
#include "LightVolumeEffects.h"

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "Device.h"
#include "Texture/Texture.h"
#include "Texture/RenderTarget.h"
#include "Texture/DepthStencil.h"
#include "Buffer/Buffer.h"

using namespace Craze;
using namespace Craze::Graphics2;

bool InjectRaysEffect::initialize()
{
	m_random = (const TextureResource*)gResMgr.loadResource(gFileDataLoader.addFile("random.png"));

	return IEffect::initialize("ScreenQuad.vsh", "RayTracing/FirstBounce.psh");
}

void InjectRaysEffect::inject(std::shared_ptr<RenderTarget> dummyTarget, std::shared_ptr<RenderTarget> RSMs[], std::shared_ptr<DepthStencil> RSMdepth, std::shared_ptr<UAVBuffer> outRays)
{
	if (!m_random->isLoaded())
	{
		return;
	}

	IEffect::set();

	ID3D11RenderTargetView* rtv = dummyTarget->GetRenderTargetView();
	ID3D11UnorderedAccessView* uav = outRays->GetUAV();
	gpDevice->GetDeviceContext()->OMSetRenderTargetsAndUnorderedAccessViews(1, &rtv, nullptr, 1, 1, &uav, 0);

	ID3D11ShaderResourceView* srvs[] = { RSMs[0]->GetResourceView(), RSMs[1]->GetResourceView(), RSMdepth->GetSRV(), m_random->m_texture->GetResourceView() };
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, srvs);

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gpDevice->GetDeviceContext()->Draw(3, 0);

	ZeroMemory(*srvs, sizeof(void*) * 4);
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, srvs);
}