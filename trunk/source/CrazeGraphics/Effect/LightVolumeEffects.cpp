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

#include "PIXHelper.h"

using namespace Craze;
using namespace Craze::Graphics2;

bool LVFirstBounceEffect::initialize()
{
	m_random = std::dynamic_pointer_cast<const TextureResource>(gResMgr.loadResource(gFileDataLoader.addFile("random.png")));

	return IEffect::initialize("ScreenQuad.vsh", "RayTracing/FirstBounce.psh");
}

void LVFirstBounceEffect::doFirstBounce(std::shared_ptr<RenderTarget> dummyTarget, std::shared_ptr<RenderTarget> RSMs[], std::shared_ptr<DepthStencil> RSMdepth, std::shared_ptr<UAVBuffer> outRays, const Matrix4& viewProj)
{
	if (!m_random.get())
	{
		return;
	}

	IEffect::set();

	CBPerLight cbLight;
	cbLight.lightViewProj = viewProj.GetInverse();
	gpDevice->GetCbuffers()->SetLight(cbLight);

	ID3D11RenderTargetView* rtv = dummyTarget->GetRenderTargetView();
	ID3D11UnorderedAccessView* uav = outRays->GetUAV();
	unsigned int initCount = 0;
	gpDevice->GetDeviceContext()->OMSetRenderTargetsAndUnorderedAccessViews(1, &rtv, nullptr, 1, 1, &uav, &initCount);
	
	ID3D11ShaderResourceView* srvs[] = { RSMs[0]->GetResourceView(), RSMs[1]->GetResourceView(), m_random.get()->GetResourceView(), RSMdepth->GetSRV() };
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, srvs);

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gpDevice->GetDeviceContext()->Draw(3, 0);

	ZeroMemory(srvs, sizeof(void*) * 4);
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, srvs);

	gpDevice->SetRenderTarget(dummyTarget, nullptr);
}


bool LVInjectRaysEffect::initialize()
{
	unsigned int args[] = { 0, 2, 0, 0};
	m_argBuffer = SRVBuffer::CreateRawArg(gpDevice, sizeof(unsigned int) * 4, args, "LVInjectRays arg buffer");

	m_tessShaders = EffectHelper::LoadShaderFromResource<TessShaderResource>("RayTracing/LineTess.tess");

	CD3D11_BLEND_DESC bDesc;
	bDesc.IndependentBlendEnable = false;
	bDesc.AlphaToCoverageEnable = false;
	bDesc.RenderTarget[0].BlendEnable = true;
	bDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	bDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	bDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	bDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;

	gpDevice->GetDevice()->CreateBlendState(&bDesc, &m_blendState);

	CD3D11_RASTERIZER_DESC rsDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, true, 0, 0.f, 0.f, true, false, false, true);
	gpDevice->GetDevice()->CreateRasterizerState(&rsDesc, &m_rasterizerState);

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

void LVInjectRaysEffect::injectRays(std::shared_ptr<UAVBuffer> rays, std::shared_ptr<RenderTarget> LVs[])
{
	IEffect::set();
	gpDevice->SetShader(m_tessShaders->m_hs);
	gpDevice->SetShader(m_tessShaders->m_ds);

	auto dc = gpDevice->GetDeviceContext();

	float bf[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	gpDevice->GetDeviceContext()->OMSetBlendState(m_blendState, bf, 0xFFFFFFFF);

	float black[] = { 0.f, 0.f, 0.f, 0.f };
	dc->ClearRenderTargetView(LVs[0]->GetRenderTargetView(), black);
	dc->ClearRenderTargetView(LVs[1]->GetRenderTargetView(), black);
	dc->ClearRenderTargetView(LVs[2]->GetRenderTargetView(), black);

	dc->RSSetState(m_rasterizerState);

	//Prepare the argument buffer for the indirect call
	dc->CopyStructureCount(m_argBuffer->GetBuffer(), 0, rays->GetUAV());

	ID3D11Buffer* vs = nullptr;
	unsigned int stride = 0;
	unsigned int offset = 0;
	dc->IASetVertexBuffers(0, 1, &vs, &stride, &offset);

	ID3D11ShaderResourceView* srv[] = { rays->GetSRV(), m_argBuffer->GetSRV() };
	gpDevice->GetDeviceContext()->VSSetShaderResources(0, 2, srv);

	gpDevice->SetRenderTargets(LVs, 3, nullptr);

	//Draw the rays
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST);
	dc->DrawInstancedIndirect(m_argBuffer->GetBuffer(), 0);

	ZeroMemory(srv, sizeof(void*) * 2);
	gpDevice->GetDeviceContext()->VSSetShaderResources(0, 2, srv);

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	gpDevice->SetShader((ID3D11GeometryShader*)nullptr);
	gpDevice->SetShader((ID3D11HullShader*)nullptr);
	gpDevice->SetShader((ID3D11DomainShader*)nullptr);

	gpDevice->GetDeviceContext()->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);
	dc->RSSetState(nullptr);
}

bool LVAmbientLightingEffect::initialize()
{
	return IEffect::initialize("ScreenQuad.vsh", "IndirectLighting.psh"); 
}

void LVAmbientLightingEffect::doLighting(std::shared_ptr<RenderTarget> LVs[], std::shared_ptr<RenderTarget> gbuffers[], ID3D11ShaderResourceView* depth, const LightVolumeInfo& LVinfo)
{
	IEffect::set();


	auto dc = gpDevice->GetDeviceContext();

	ID3D11ShaderResourceView* srvs[] = { gbuffers[0]->GetResourceView(), gbuffers[1]->GetResourceView(), nullptr, depth, LVs[0]->GetResourceView(), LVs[1]->GetResourceView(), LVs[2]->GetResourceView() };

	dc->PSSetShaderResources(0, 7, srvs);
	dc->Draw(3, 0);

	ZeroMemory(srvs, sizeof(void*) * 7);
	dc->PSSetShaderResources(0, 7, srvs);
}