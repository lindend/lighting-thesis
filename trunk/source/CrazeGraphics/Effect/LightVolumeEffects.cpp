#include "CrazeGraphicsPCH.h"
#include "LightVolumeEffects.h"

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "Device.h"
#include "Texture/Texture.h"
#include "Texture/RenderTarget.h"
#include "Texture/DepthStencil.h"
#include "Buffer/Buffer.h"
#include "EffectUtil/EffectHelper.h"
#include "EffectUtil/CBufferHelper.hpp"

using namespace Craze;
using namespace Craze::Graphics2;

bool LVFirstBounceEffect::initialize()
{
	m_random = (const TextureResource*)gResMgr.loadResource(gFileDataLoader.addFile("random.png"));

	return IEffect::initialize("ScreenQuad.vsh", "RayTracing/FirstBounce.psh");
}

void LVFirstBounceEffect::doFirstBounce(std::shared_ptr<RenderTarget> dummyTarget, std::shared_ptr<RenderTarget> RSMs[], std::shared_ptr<DepthStencil> RSMdepth, std::shared_ptr<UAVBuffer> outRays)
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

	ZeroMemory(srvs, sizeof(void*) * 4);
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, srvs);
}


bool LVInjectRaysEffect::initialize()
{
	unsigned int args[] = { 0, 1, 0, 0};
	m_argBuffer = Buffer::CreateArg(gpDevice, sizeof(unsigned int) * 4, args, "LVInjectRays arg buffer");

	m_tessShaders = EffectHelper::LoadShaderFromResource<TessShaderResource>("RayTracing/LineTess.tess");

	m_cbuffer = EffectHelper::CreateConstantBuffer(gpDevice, sizeof(Vector4) * 3);

	return IEffect::initialize("RayTracing/LightInject.vsh", "RayTracing/RasterizeSH.psh", "RayTracing/LineDraw.gsh");
}

const D3D11_INPUT_ELEMENT_DESC* LVInjectRaysEffect::getLayout(int& count)
{
	count = 2;

	static const D3D11_INPUT_ELEMENT_DESC pDesc[] = 
	{
		{"POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	return pDesc;
}

void LVInjectRaysEffect::injectRays(std::shared_ptr<UAVBuffer> rays, std::shared_ptr<RenderTarget> LVs[], const LightVolumeInfo& LVinfo)
{
	IEffect::set();
	gpDevice->SetShader(m_tessShaders->m_hs);
	gpDevice->SetShader(m_tessShaders->m_ds);

	CBufferHelper cbuffer(gpDevice, m_cbuffer);
	cbuffer[0] = LVinfo.start.v;
	cbuffer[1] = LVinfo.end.v;
	cbuffer[2] = Vector4(LVinfo.cellSize, LVinfo.numCells);
	cbuffer.Unmap();

	auto dc = gpDevice->GetDeviceContext();

	dc->HSSetConstantBuffers(0, 1, &m_cbuffer);
	dc->DSSetConstantBuffers(0, 1, &m_cbuffer);
	dc->GSSetConstantBuffers(0, 1, &m_cbuffer);

	//Prepare the argument buffer for the indirect call
	dc->CopyStructureCount(m_argBuffer->GetBuffer(), 0, rays->GetUAV());

	ID3D11Buffer* vs = rays->GetBuffer();
	unsigned int stride = sizeof(Vector4) * 4;
	unsigned int offset = 0;
	dc->IASetVertexBuffers(0, 1, &vs, &stride, &offset);

	gpDevice->SetRenderTargets(LVs, 3, nullptr);

	//Draw the rays
	dc->DrawInstancedIndirect(m_argBuffer->GetBuffer(), 0);

	gpDevice->SetShader((ID3D11HullShader*)nullptr);
	gpDevice->SetShader((ID3D11DomainShader*)nullptr);
}