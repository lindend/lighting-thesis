#include "CrazeGraphicsPCH.h"
#include "Renderer.h"

#include <sstream>

#include "Intersection/Frustum.h"

#include "../Graphics.h"
#include "../Device.h"
#include "../DrawList.h"
#include "../Scene/Scene.h"
#include "../Light/SpotLight.h"
#include "../Light/DirectionalLight.h"
#include "../Light/Light.h"
#include "Texture/RenderTarget.h"
#include "Texture/DepthStencil.h"
#include "../Mesh.h"
#include "Effect/ShadowEffect.h"
#include "Effect/GBufferEffect.h"
#include "Effect/LightAccumulationEffect.h"
#include "Effect/ShadingEffect.h"
#include "Effect/FontEffect.h"
#include "Effect/ShadowEffect.h"
#include "Effect/AmbientLighting.h"
#include "Effect/CSLighting.h"
#include "Effect/LightingEffect.h"
#include "Effect/DebugDrawEffect.h"
#include "Effect/CopyToBackBuffer.h"
#include "Buffer/Buffer.h"
#include "Renderer/DrawRays.h"

#include "PIXHelper.h"

using namespace Craze::Graphics2;
using namespace Craze;

namespace Craze
{
	namespace Graphics2
	{
		Renderer* gpRenderer = nullptr;
	}
}

Renderer::Renderer()
{
	m_pLightPassDSS = 0;
	m_pShadingPassDSS = 0;
	m_pSamplerState = 0;
	m_pPointSampler = 0;
	m_pPointBorderSampler = 0;
	m_pBilinearBorderSampler = 0;
	m_pShadowMap = nullptr;
	m_pFontFace = 0;
}

Renderer::~Renderer()
{
	Shutdown();
}

void Renderer::Initialize()
{
	Vector2 viewPort = gpDevice->GetViewPort();

	int vpx = (int)viewPort.x, vpy = (int)viewPort.y;
	m_GBuffers[0] = RenderTarget::Create2D(gpDevice, vpx, vpy, 1, TEXTURE_FORMAT_COLOR_LINEAR, "GBuffer color specular");
	//Create the normal specular rendertarget
	m_GBuffers[1] = RenderTarget::Create2D(gpDevice, vpx, vpy, 1, TEXTURE_FORMAT_VECTOR4, "GBuffer normal roughness");
	m_GBuffers[2] = RenderTarget::Create2D(gpDevice, vpx, vpy, 1, TEXTURE_FORMAT_COLOR_LINEAR, "GBuffer indirect illumination");

	m_pOutputTarget = RenderTarget::Create2D(gpDevice, vpx, vpy, 1, TEXTURE_FORMAT_HALFVECTOR4, "Output target", true);

	m_pShadowMap = RenderTarget::Create2D(gpDevice, 4096, 4096, 1, TEXTURE_FORMAT_FLOAT, "Shadow map");

	m_rsmTargets[0] = RenderTarget::Create2D(gpDevice, 128, 128, 1, TEXTURE_FORMAT_COLOR_LINEAR, "RSM color");
	m_rsmTargets[1] = RenderTarget::Create2D(gpDevice, 128, 128, 1, TEXTURE_FORMAT_VECTOR4, "RSM normal");

	m_pShadowDS = DepthStencil::Create2D(gpDevice, 128, 128, DEPTHSTENCIL_FORMAT_D24S8);

	m_pScreenQuad = Mesh::createScreenQuad(gpDevice);

	/*m_pSpotLight.reset(new SpotLight(pDevice));
	m_pSpotLight->SetDiffuse(Vector3::ONE);
	Vector3 pos = Vector3(2.0f, 2.0f, -2.0f) * 2.0f;
	m_pSpotLight->SetPosition(pos);
	pos.Normalize();
	m_pSpotLight->SetDirection(pos * -1.0f, Vector3::UP);
	m_pSpotLight->SetProjection(PI / 5.0f, 90.0f, 0.1f);
	m_pSpotLight->SetSpecular(0.5f);*/

	m_pDirLight.reset(CrNew DirectionalLight());
	m_pDirLight->SetDiffuse(Vector3::ONE * 2.f);
	m_pDirLight->SetDirection(Vector3::Normalize(Vector3(1.6f, -2.0f, -1.6f)), Vector3::UP);
	m_pDirLight->SetSpecular(0.5f);

	D3D11_DEPTH_STENCIL_DESC dsdesc;
	ZeroMemory(&dsdesc, sizeof(dsdesc));
	dsdesc.DepthEnable = true;
	dsdesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsdesc.StencilEnable = false;
	
	gpDevice->GetDevice()->CreateDepthStencilState(&dsdesc, &m_pLightPassDSS);

	dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	gpDevice->GetDevice()->CreateDepthStencilState(&dsdesc, &m_pShadingPassDSS);

	dsdesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	dsdesc.DepthEnable = false;
	gpDevice->GetDevice()->CreateDepthStencilState(&dsdesc, &m_pNoDepthDSS);

	D3D11_SAMPLER_DESC smDesc;
	ZeroMemory(&smDesc, sizeof(smDesc));
	smDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	smDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	smDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	smDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	smDesc.MinLOD = -D3D11_FLOAT32_MAX;
	smDesc.MaxLOD = D3D11_FLOAT32_MAX;
	smDesc.MaxAnisotropy = 16;
	smDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	gpDevice->GetDevice()->CreateSamplerState(&smDesc, &m_pSamplerState);
	gpDevice->GetDeviceContext()->PSSetSamplers(0, 1, &m_pSamplerState);
	gpDevice->GetDeviceContext()->VSSetSamplers(0, 1, &m_pSamplerState);
	gpDevice->GetDeviceContext()->GSSetSamplers(0, 1, &m_pSamplerState);

	smDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

	gpDevice->GetDevice()->CreateSamplerState(&smDesc, &m_pPointSampler);
	gpDevice->GetDeviceContext()->PSSetSamplers(3, 1, &m_pPointSampler);
	gpDevice->GetDeviceContext()->VSSetSamplers(3, 1, &m_pPointSampler);
	gpDevice->GetDeviceContext()->GSSetSamplers(3, 1, &m_pPointSampler);

	smDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	smDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	smDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

	gpDevice->GetDevice()->CreateSamplerState(&smDesc, &m_pPointBorderSampler);
	gpDevice->GetDeviceContext()->PSSetSamplers(4, 1, &m_pPointBorderSampler);
	gpDevice->GetDeviceContext()->VSSetSamplers(4, 1, &m_pPointBorderSampler);
	gpDevice->GetDeviceContext()->GSSetSamplers(4, 1, &m_pPointBorderSampler);

	smDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	gpDevice->GetDevice()->CreateSamplerState(&smDesc, &m_pBilinearBorderSampler);
	gpDevice->GetDeviceContext()->PSSetSamplers(2, 1, &m_pBilinearBorderSampler);
	gpDevice->GetDeviceContext()->VSSetSamplers(2, 1, &m_pBilinearBorderSampler);
	gpDevice->GetDeviceContext()->GSSetSamplers(2, 1, &m_pBilinearBorderSampler);

	D3D11_BLEND_DESC bdesc;
	ZeroMemory(&bdesc, sizeof(bdesc));
	bdesc.AlphaToCoverageEnable = false;
	bdesc.IndependentBlendEnable = false;
	bdesc.RenderTarget[0].BlendEnable = true;
	bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bdesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	gpDevice->GetDevice()->CreateBlendState(&bdesc, &m_pLightBS);

	//m_pFontFace = gFontMgr.CreateFace("Katana", "Regular", 12);
	//m_Str = m_pFontFace->BuildStaticString("Hello world", Vector3::ONE);

	m_lightVolumeInjector.initialize();

	m_rayDrawer = new DrawRays();
	m_rayDrawer->initialize();
}

void Renderer::Shutdown()
{
	m_lightVolumeInjector.destroy();

	for (unsigned int i = 0; i < NumGBuffers; ++i)
	{
		m_GBuffers[i] = nullptr;
	}

	delete m_pFontFace;
	m_pFontFace = 0;

	SAFE_RELEASE(m_pLightBS);
	SAFE_RELEASE(m_pBilinearBorderSampler);
	SAFE_RELEASE(m_pPointBorderSampler);
	SAFE_RELEASE(m_pPointSampler);
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pNoDepthDSS);
	SAFE_RELEASE(m_pShadingPassDSS);
	SAFE_RELEASE(m_pLightPassDSS);
}

void Renderer::InitFrame(Scene* pScene, CBPerFrame &cbuffer)
{
	const Camera* pCam = pScene->getCamera();
	Matrix4 viewProj = pCam->GetView() * pCam->GetProjection();

	cbuffer.cameraPos = pCam->GetPosition();
	cbuffer.viewProj = viewProj;
	cbuffer.ambientColor = pScene->AmbientLight;
	
	LightVolumeInfo lvInfo = m_lightVolumeInjector.getLVInfo(pCam);
	cbuffer.LVStart = lvInfo.start;
	cbuffer.LVEnd = lvInfo.end;
	cbuffer.LVCellSize = Vector4(lvInfo.cellSize, lvInfo.numCells);
}

void Renderer::RenderScene(Craze::Graphics2::Scene* pScene)
{
	PROFILEF();

	static DrawList mainScene;
	static DrawList shadowScene;
	mainScene.clear();
	shadowScene.clear();

	pScene->addSun(createDirectionalLight(Vector3(0.2, -1.f, 0.2f), Vector3::ONE));

	pScene->update();

	//If we should redraw the UI, begin creating a deferred device context with the rendering for it now

	const Camera* pCam = pScene->getCamera();
	Matrix4 viewProj = pCam->GetView() * pCam->GetProjection();

	Vector3 pos = pCam->GetPosition() + pCam->GetDirection() * pCam->GetFar() * 0.5f - m_pDirLight->GetDirection() * pCam->GetFar() * 2.f;
	Matrix4 lightViewProj = Matrix4::CreateView(pos, m_pDirLight->GetDirection() + pos, Vector3::UP) * Matrix4::CreateOrtho(pCam->GetFar(), pCam->GetFar(), 10.f, pCam->GetFar() * 4.f);

	std::shared_ptr<RenderTarget>* lightVolumes = m_lightVolumeInjector.getLightingVolumes(pScene);

	//Thread this...
	pScene->buildDrawList(&mainScene, viewProj);

	//Move this somewhere else
	pScene->buildDrawList(&shadowScene, lightViewProj);

	LightArray visibleLights = pScene->getVisibleLights(viewProj);
	Transform(pCam->GetView(), visibleLights.pPositions, visibleLights.numPosSoA);

	float color[] = {0.5f, 0.5f, 0.5f, 0.f};
	gpDevice->GetDeviceContext()->ClearRenderTargetView(m_GBuffers[0]->GetRenderTargetView(), color);
	gpDevice->Clear(Vector4(0.5f, 0.5f, 0.5f, 1.0f));

	/*if (!gpGraphics->Params.DisableDirect)
	{
		gpDevice->GetDeviceContext()->ClearDepthStencilView(m_pShadowDS->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);
		gpDevice->SetRenderTarget(m_pShadowMap, m_pShadowDS);

		gpDevice->GetCbuffers()->SetLight(*m_pDirLight, lightViewProj, pos);

		gFxShadow.set();

		for (auto i = shadowScene.begin(); i != shadowScene.end(); ++i)
		{
			gFxShadow.setObjectProperties(*i->second.m_transform, *i->second.m_material);
			i->second.m_mesh->draw();
		}
	}*/

	gpDevice->SetRenderTargets(m_GBuffers, NumGBuffers, gpDevice->GetDefaultDepthBuffer());
	gpDevice->ClearCache();
	{
		PIXMARKER(L"Create g-buffers");
		gFxGBuffer.set(pCam);
	
		//Should be left as it is? Probably already got a few other threads running
		//Make sure to wait for the mainScene draw list to be ready here first
		for (auto i = mainScene.begin(); i != mainScene.end(); ++i)
		{
			gFxGBuffer.setObjectProperties(*i->second.m_transform, *i->second.m_material);
			i->second.m_mesh->draw();
		}
	}
	
	Light dir = pScene->getSun();


	//The gbuffers are ready, perform lighting
	ID3D11ShaderResourceView* pSRVs[4] = { m_GBuffers[0]->GetResourceView(), m_GBuffers[1]->GetResourceView(), m_GBuffers[2]->GetResourceView(), gpDevice->GetDefaultDepthSRV() };
	//gFxCSLighting.run(pCam, pSRVs, m_pOutputTarget->GetUAV(), visibleLights);

	//Do the other lights, with shadows
	gpDevice->SetRenderTarget(m_pOutputTarget, nullptr);
	float black[] = { 0.f, 0.f, 0.f, 0.f };
	gpDevice->GetDeviceContext()->ClearRenderTargetView(m_pOutputTarget->GetRenderTargetView(), black);

	const float bf[4] = {1.f, 1.f, 1.f, 1.f};
	gpDevice->GetDeviceContext()->OMSetBlendState(m_pLightBS, bf, 0xFFFFFFFF);

	{
		PIXMARKER(L"Do lighting");
		ID3D11ShaderResourceView* pOutSRVs[] = { nullptr, nullptr };
		gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, pSRVs);
		gpDevice->GetDeviceContext()->PSSetShaderResources(4, 2, pOutSRVs);

		gFxLighting.doLighting(dir, pCam->GetView(), nullptr);
	}
	
	gFxLVAmbientLighting.doLighting(lightVolumes, m_GBuffers, gpDevice->GetDefaultDepthSRV(), m_lightVolumeInjector.getLVInfo(pCam));

	/*gFxAmbientLighting.set();
	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gpDevice->GetDeviceContext()->Draw(3, 0);*/

	pSRVs[0] = nullptr;
	pSRVs[1] = nullptr;
	pSRVs[2] = nullptr;
	pSRVs[3] = nullptr;
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, pSRVs);
	gpDevice->GetDeviceContext()->PSSetShaderResources(4, 2, pSRVs);

	gpDevice->GetDeviceContext()->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);

	gpDevice->SetRenderTargets(&m_pOutputTarget, 1, gpDevice->GetDefaultDepthBuffer());

	{
		PIXMARKER(L"Draw rays");
		m_rayDrawer->render(m_lightVolumeInjector.getCollidedRays(), viewProj);
	}

	gFxCopyToBack.doCopy(m_pOutputTarget);

	gpDevice->GetDeviceContext()->OMSetDepthStencilState(0, 0);

}

void Renderer::BindScene(Scene* pScene)
{
	//m_ISPMRenderer.BindScene(pScene);
}

void Renderer::RenderLight(DirectionalLight* pLight, Scene* pScene)
{
	
}
