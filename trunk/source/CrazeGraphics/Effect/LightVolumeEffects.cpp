#include "CrazeGraphicsPCH.h"
#include "LightVolumeEffects.h"
#include "Renderer/LightVolumeInjector.h"

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "Device.h"
#include "Texture/Texture.h"
#include "Texture/RenderTarget.h"
#include "Texture/DepthStencil.h"
#include "Buffer/Buffer.h"
#include "EffectUtil/EffectHelper.h"
#include "EffectUtil/CBufferHelper.hpp"
#include "Scene/Scene.h"

#include "PIXHelper.h"

using namespace Craze;
using namespace Craze::Graphics2;

bool LVFirstBounceEffect::initialize()
{
	m_random = std::dynamic_pointer_cast<const TextureResource>(gResMgr.loadResource(gFileDataLoader.addFile("random.png")));
	m_frustumCBuffer = EffectHelper::CreateConstantBuffer(gpDevice, sizeof(Vector4) * 8);

	return IEffect::initialize("ScreenQuad.vsh", "RayTracing/FirstBounce.psh");
}

void expand(const Vector3* vs, Vector3* vout, int i0, int i1, int i2, int i3, float len)
{
	Vector3 d0 = Normalize(vs[i1] - vs[i0]);
	Vector3 d1 = Normalize(vs[i2] - vs[i0]);
	Vector3 d2 = Normalize(vs[i3] - vs[i0]);

	vout[i0] = (d0 + d1 + d2) * -len;
	vout[i1] = vout[i1] + d0 * len;
	vout[i2] = vout[i2] + d1 * len;
	vout[i3] = vout[i3] + d2 * len;
}

void LVFirstBounceEffect::doFirstBounce(std::shared_ptr<RenderTarget> dummyTarget, std::shared_ptr<RenderTarget> RSMs[], std::shared_ptr<DepthStencil> RSMdepth, std::shared_ptr<UAVBuffer> outRays, const Matrix4& viewProj, const Camera* cam, bool first)
{
	if (!m_random.get())
	{
		return;
	}

	Vector3 corners[8];
	cam->GetFrustumCorners(1.f, 2000.f, corners);

	Vector3 cornerAdjustment[8];
	ZeroMemory(cornerAdjustment, sizeof(Vector3) * 8);
	float len = 600.f;
	expand(corners, cornerAdjustment, 0, 1, 3, 4, len);
	expand(corners, cornerAdjustment, 2, 1, 3, 6, len);

	expand(corners, cornerAdjustment, 5, 1, 5, 6, len);
	expand(corners, cornerAdjustment, 7, 3, 4, 6, len);
	
	for (int i = 0; i < 8; ++i)
	{
		corners[i] = corners[i] + cornerAdjustment[i];
	}

	CBufferHelper cb(gpDevice, m_frustumCBuffer);
	for (int i = 0; i < 8; ++i)
	{
		cb[i] = corners[i].v;
	}
	cb.Unmap();
	gpDevice->GetDeviceContext()->PSSetConstantBuffers(1, 1, &m_frustumCBuffer);

	IEffect::set();

	CBPerLight cbLight;
	cbLight.lightViewProj = viewProj.GetInverse();
	gpDevice->GetCbuffers()->SetLight(cbLight);

	ID3D11RenderTargetView* rtv = dummyTarget->GetRenderTargetView();
	ID3D11UnorderedAccessView* uav = outRays->GetAppendConsumeUAV();
	unsigned int initCount = first ? 0 : -1;
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
	unsigned int args[] = { 0, 1, 0, 0};
	m_argBuffer = SRVBuffer::CreateRawArg(gpDevice, sizeof(unsigned int) * 4, args, "LVInjectRays arg buffer");

	m_tessShaders = EffectHelper::LoadShaderFromResource<TessShaderResource>("RayTracing/LineTess.tess");

	return IEffect::initialize("RayTracing/InjectTessellated.vsh",
#ifdef CRAZE_USE_SH_LV
		"RayTracing/RasterizeSH.psh",
#else
		"RayTracing/RasterizeRaysCM.psh",
#endif
		"RayTracing/InjectTessellated.gsh");
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
	//gpDevice->SetShader(m_tessShaders->m_hs);
	//gpDevice->SetShader(m_tessShaders->m_ds);

	auto dc = gpDevice->GetDeviceContext();

	//Prepare the argument buffer for the indirect call
	dc->CopyStructureCount(m_argBuffer->GetBuffer(), 0, rays->GetAppendConsumeUAV());

	ID3D11Buffer* vs = nullptr;
	unsigned int stride = 0;
	unsigned int offset = 0;
	dc->IASetVertexBuffers(0, 1, &vs, &stride, &offset);

	ID3D11ShaderResourceView* srv = rays->GetSRV();
	gpDevice->GetDeviceContext()->VSSetShaderResources(0, 1, &srv);

	gpDevice->SetRenderTargets(LVs, CRAZE_NUM_LV, nullptr);

	//Draw the rays
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	//dc->Draw(515000, 0);
	dc->DrawInstancedIndirect(m_argBuffer->GetBuffer(), 0);

	srv = nullptr;
	gpDevice->GetDeviceContext()->VSSetShaderResources(0, 1, &srv);

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	IEffect::reset();

	//gpDevice->SetShader((ID3D11GeometryShader*)nullptr);
	//gpDevice->SetShader((ID3D11HullShader*)nullptr);
	//gpDevice->SetShader((ID3D11DomainShader*)nullptr);
}

bool LVAmbientLightingEffect::initialize()
{
	return IEffect::initialize("ScreenQuad.vsh", "IndirectLighting.psh"); 
}

void LVAmbientLightingEffect::doLighting(std::shared_ptr<RenderTarget> LVs[], std::shared_ptr<RenderTarget> gbuffers[], ID3D11ShaderResourceView* depth, const LightVolumeInfo& LVinfo)
{
	IEffect::set();


	auto dc = gpDevice->GetDeviceContext();

	ID3D11ShaderResourceView* srvs[10];
	memset(srvs, 0, sizeof(nullptr) * 10);
	srvs[0] = gbuffers[0]->GetResourceView();
	srvs[1] = gbuffers[1]->GetResourceView();
	srvs[3] = depth;
	
	for (int i = 0; i < CRAZE_NUM_LV; ++i)
	{
		srvs[4 + i] = LVs[i]->GetResourceView();
	}

	dc->PSSetShaderResources(0, 10, srvs);
	dc->Draw(3, 0);

	ZeroMemory(srvs, sizeof(void*) * 10);
	dc->PSSetShaderResources(0, 10, srvs);
}