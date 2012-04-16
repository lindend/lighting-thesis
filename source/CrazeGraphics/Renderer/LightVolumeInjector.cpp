#include "CrazeGraphicsPCH.h"
#include "LightVolumeInjector.h"

#include <memory>

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "Device.h"
#include "Graphics.h"
#include "Renderer/Renderer.h"

#include "Buffer/Buffer.h"
#include "Scene/Scene.h"
#include "Texture/RenderTarget.h"
#include "Texture/DepthStencil.h"
#include "DrawList.h"

#include "EffectUtil/EffectHelper.h"
#include "EffectUtil/ShaderResource.h"

#include "Effect/GBufferEffect.h"
#include "Effect/LightVolumeEffects.h"

#include "Model.h"
#include "Geometry/MeshData.h"
#include "GPUProfiler/GPUProfiler.h"

#include "PIXHelper.h"


using namespace Craze;
using namespace Craze::Graphics2;

bool LightVolumeInjector::initialize()
{
	m_fxFirstBounce.reset(new LVFirstBounceEffect());
	m_fxInjectRays.reset(new LVInjectRaysEffect());
	m_fxMergeLV.reset(new IEffect());

	m_fxFirstBounce->initialize();
	m_fxInjectRays->initialize();
	m_fxMergeLV->initialize("RayTracing/MergeLVs.vsh", "RayTracing/MergeLVs.psh", "RayTracing/MergeLVs.gsh");

	m_RSMs[0] = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_COLOR_LINEAR, "RSM color");
	m_RSMs[1] = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_VECTOR4, "RSM normal");

	m_RSMDS = DepthStencil::Create2D(gpDevice, RSMResolution, RSMResolution, DEPTHSTENCIL_FORMAT_D24S8);

	m_dummy = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_8BIT_UNORM, "Dummy light volume render target");

	for (int i = 0; i < CRAZE_NUM_LV; ++i)
	{
		m_lightingVolumes[i] = RenderTarget::Create3D(gpDevice, LightVolumeResolution, LightVolumeResolution, LightVolumeResolution, 1, TEXTURE_FORMAT_HALFVECTOR4, "Light volume");
		m_targetLightVolumes[i] = RenderTarget::Create3D(gpDevice, LightVolumeResolution, LightVolumeResolution, LightVolumeResolution, 1, TEXTURE_FORMAT_HALFVECTOR4, "Target light volume");
		float black[] = { 0.f, 0.f, 0.f, 0.f};
		gpDevice->GetDeviceContext()->ClearRenderTargetView(m_targetLightVolumes[i]->GetRenderTargetView(), black);
	}

	m_toTestRays = UAVBuffer::Create(gpDevice, sizeof(float) * 3 * 3, MaxPhotonRays, true, "To test rays");
	m_collidedRays = UAVBuffer::Create(gpDevice, sizeof(float) * 3 * 3, MaxPhotonRays, true, "Collided rays");

	m_rayTraceCS = EffectHelper::LoadShaderFromResource<ComputeShaderResource>("RayTracing/RayTrace.csh");

	//Hard coded collision geometry!
	std::shared_ptr<const Model> triMesh = std::dynamic_pointer_cast<const Model>(gResMgr.loadResourceBlocking(gFileDataLoader.addFile("Sponza/sponza_low.crm")));
	std::tr1::shared_ptr<MeshData> meshData = triMesh->getMeshes()[0].mesh->getMeshData();
	const Vertex* verts = meshData->GetPosNormalUv();
	const unsigned short* indices = meshData->GetIndices();
	Vec3* tris = new Vec3[meshData->GetNumIndices()];
	for (int i = 0; i < meshData->GetNumIndices(); ++i)
	{
		tris[i] = verts[indices[i]].position;
	}
	setTriangles(tris, meshData->GetNumIndices() / 3);
	delete [] tris;

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
	gpDevice->GetDevice()->CreateBlendState(&bDesc, &m_addBS);

	bDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	gpDevice->GetDevice()->CreateBlendState(&bDesc, &m_mergeBS);

	D3D11_RASTERIZER_DESC rsDesc;//(D3D11_FILL_SOLID, D3D11_CULL_NONE, true, 0, 0.f, 0.f, true, false, false, true);
	rsDesc.AntialiasedLineEnable = false;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.f;
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.FrontCounterClockwise = true;
	rsDesc.MultisampleEnable = false;
	rsDesc.ScissorEnable = false;
	rsDesc.SlopeScaledDepthBias = 0.f;
	gpDevice->GetDevice()->CreateRasterizerState(&rsDesc, &m_AALinesRS);

	D3D11_SAMPLER_DESC smDesc;
	ZeroMemory(&smDesc, sizeof(smDesc));
	smDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	smDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	smDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	smDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	smDesc.MinLOD = 9.f;
	smDesc.MaxLOD = D3D11_FLOAT32_MAX;
	smDesc.MaxAnisotropy = 16;
	smDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	gpDevice->GetDevice()->CreateSamplerState(&smDesc, &m_lowMipSampler);
	gpDevice->GetDeviceContext()->PSSetSamplers(0, 1, &m_lowMipSampler);
	gpDevice->GetDeviceContext()->VSSetSamplers(0, 1, &m_lowMipSampler);
	gpDevice->GetDeviceContext()->GSSetSamplers(0, 1, &m_lowMipSampler);

	return true;
}

void LightVolumeInjector::setTriangles(Vec3* tris, int numTris)
{
	m_triangleBuffer = SRVBuffer::CreateStructured(gpDevice, sizeof(float) * 3 * 3, numTris, tris, false, "Triangle buffer");
	m_numTriangles = numTris;
}

Camera findSMCamera(const DirectionalLight& l, Scene* scene)
{
	Camera c;
	
	Vector3 lpos = Vector3(l.dir.x, l.dir.y, l.dir.z);
	lpos = Normalize(lpos);
	lpos*=-1;
	lpos *= 2000;
	c.SetPosition(lpos);

	c.SetDirection(l.dir);
	if (Dot(c.GetDirection(), Vector3::UP) > 0.9f)
	{
		c.SetUp(Vector3::RIGHT);
	} else
	{
		c.SetUp(Vector3::UP);
	}
	
	c.SetOrthoProjection(2000.f, 2000.f, 10.f, 10000.f);
	return c;
}

Camera findSMCamera(const SpotLight& l, Scene* scene)
{
	return Camera();
}

std::shared_ptr<UAVBuffer> LightVolumeInjector::getCollidedRays()
{
	return m_toTestRays;
	return m_collidedRays;
}

template<typename T> void findSMCams(T lights, int numLights, Scene* scene, Camera* outCams)
{
	for (int i = 0; i < numLights; ++i)
	{
		outCams[i] = findSMCamera(lights[i], scene);
	}
}

void LightVolumeInjector::injectAll(const Camera* cams, int numCams, Scene* scene)
{
	for (int i = 0; i < numCams; ++i)
	{
		const Camera& c = cams[i];
		Matrix4 viewProj = c.GetView() * c.GetProjection();

		int prof = gpGraphics->m_profiler->beginBlock("Create RSM");
		renderRSMs(scene, &c, viewProj);
		gpGraphics->m_profiler->endBlock(prof);

		prof = gpGraphics->m_profiler->beginBlock("Spawn rays");
		spawnRays(viewProj, scene->getCamera());
		gpGraphics->m_profiler->endBlock(prof);

		prof = gpGraphics->m_profiler->beginBlock("Trace rays");
		traceRays();
		gpGraphics->m_profiler->endBlock(prof);

		prof = gpGraphics->m_profiler->beginBlock("Inject into LV");
		injectToLV(scene->getCamera());
		gpGraphics->m_profiler->endBlock(prof);
	}
}

std::shared_ptr<RenderTarget>* LightVolumeInjector::getLightingVolumes(Scene* scene)
{
	PIXMARKER(L"Create lighting volumes");

	float black[] = { 0.f, 0.f, 0.f, 0.f };
	for (int i = 0; i < CRAZE_NUM_LV; ++i)
	{
		gpDevice->GetDeviceContext()->ClearRenderTargetView(m_lightingVolumes[i]->GetRenderTargetView(), black);
	}

	MEM_AUTO_MARK_STACK;

	int numDirLights;
	const DirectionalLight* lights = scene->getDirectionalLights(numDirLights);
	Camera* dirCams = (Camera*)gMemory.StackAlloc(Align(16), sizeof(Camera) * numDirLights);
	findSMCams(lights, numDirLights, scene, dirCams);
	injectAll(dirCams, numDirLights, scene);

	const Camera* cam = scene->getCamera();
	const SpotLightArray spotLights = scene->getVisibleSpotLights(cam->GetView() * cam->GetProjection());
	Camera* spotCams = (Camera*)gMemory.StackAlloc(Align(16), sizeof(Camera) * spotLights.numLights);
	findSMCams(spotLights.spotLights, spotLights.numLights, scene, spotCams);
	injectAll(spotCams, spotLights.numLights, scene);
	
	mergeToTarget();

	return m_targetLightVolumes;
}

void LightVolumeInjector::renderRSMs(Scene* scene, const Camera* c, const Matrix4& viewProj)
{
	PIXMARKER(L"Render RSMs");

	static DrawList shadowScene;
	shadowScene.clear();
	scene->buildDrawList(&shadowScene, viewProj);

	gpDevice->SetRenderTargets(m_RSMs, 2, m_RSMDS->GetDepthStencilView());
	gpDevice->GetDeviceContext()->ClearDepthStencilView(m_RSMDS->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);

	gFxGBuffer.set(c);

	for (auto i = shadowScene.begin(); i != shadowScene.end(); ++i)
	{
		gFxGBuffer.setObjectProperties(*i->second.m_transform, *i->second.m_material);
		i->second.m_mesh->draw();
	}
}

void LightVolumeInjector::spawnRays(const Matrix4& viewProj, const Camera* cam)
{
	PIXMARKER(L"Spawn rays");
	gpDevice->GetDeviceContext()->PSSetSamplers(0, 1, &m_lowMipSampler);
	m_fxFirstBounce->doFirstBounce(m_dummy, m_RSMs, m_RSMDS, m_toTestRays, viewProj, cam);
	auto sampler = m_renderer->getBilinearSampler();
	gpDevice->GetDeviceContext()->PSSetSamplers(0, 1, &sampler);
}

/*void injectToLightVolumes()
{
	if ( FAILED( pD3DDevice->CreateInputLayout( layout,
                                            numElements,
                                            PassDesc.pIAInputSignature,
                                            PassDesc.IAInputSignatureSize,
                                            &pVertexLayout ) ) ) return fatalError("Could not create Input 
											Layout!");


}*/

void LightVolumeInjector::traceRays()
{
	PIXMARKER(L"Trace rays");
	ID3D11ShaderResourceView* triSrv = m_triangleBuffer->GetSRV();
	gpDevice->GetDeviceContext()->CSSetShaderResources(0, 1, &triSrv);

	ID3D11UnorderedAccessView* UAVs[] = { m_toTestRays->GetUAV(), m_collidedRays->GetUAV() };
	u32 initCounts[] = { -1, 0 };
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 2, UAVs, initCounts);

	gpDevice->GetDeviceContext()->CSSetShader(m_rayTraceCS->m_shader, nullptr, 0);
	gpDevice->GetDeviceContext()->Dispatch(MaxPhotonRays / (32 * 8), 1, 1);

	ZeroMemory(UAVs, sizeof(void*) * 2);
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 2, UAVs, initCounts);
}

void LightVolumeInjector::injectToLV(const Camera* cam)
{
	PIXMARKER(L"Inject rays to light volume");
	LightVolumeInfo lvinfo = getLVInfo(cam);

	float bf[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	gpDevice->GetDeviceContext()->OMSetBlendState(m_addBS, bf, 0xFFFFFFFF);
	gpDevice->GetDeviceContext()->RSSetState(m_AALinesRS);

	m_fxInjectRays->injectRays(m_toTestRays, m_lightingVolumes);

	gpDevice->GetDeviceContext()->RSSetState(nullptr);
}

void LightVolumeInjector::mergeToTarget()
{
	PIXMARKER(L"Merge to target light volumes");
	m_fxMergeLV->set();

	float bf[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	gpDevice->GetDeviceContext()->OMSetBlendState(m_mergeBS, bf, 0xFFFFFFFF);

	ID3D11ShaderResourceView* srvs[CRAZE_NUM_LV];
	for (int i = 0; i < CRAZE_NUM_LV; ++i)
	{
		srvs[i] = m_lightingVolumes[i]->GetResourceView();
	}
	
	gpDevice->SetRenderTargets(m_targetLightVolumes, CRAZE_NUM_LV, nullptr);
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, CRAZE_NUM_LV, srvs);
	gpDevice->GetDeviceContext()->DrawInstanced(3, LightVolumeResolution, 0, 0);
	m_fxMergeLV->reset();

	gpDevice->SetRenderTarget(nullptr);

	gpDevice->GetDeviceContext()->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);
}

const LightVolumeInfo LightVolumeInjector::getLVInfo(const Camera* cam) const
{
	LightVolumeInfo lvinfo;
	
	float zSlice = 2000.f;
	
	lvinfo.start = cam->GetPosition();
	lvinfo.end = cam->GetPosition();

	Vector3 corners[8];
	cam->GetFrustumCorners(2.f, zSlice, corners);
	

	float size = 0.f;
	Vector3 minCorner = corners[0];
	for (int i = 0; i < 8; ++i)
	{
		for (int j = i + 1; j < 8; ++j)
		{
			size = Max(size, LengthSquared(corners[i] - corners[j]));
		}
		minCorner = Min(minCorner, corners[i]);
	}
	size = Sqrt(size);
	float cellSize = size / (float)LightVolumeResolution;

	lvinfo.start = minCorner;
	lvinfo.end = lvinfo.start + Vector3(size);

	Vector3 cellStart = Floor(lvinfo.start / cellSize);
	Vector3 snapStart = cellStart * cellSize;
	lvinfo.start = snapStart;
	lvinfo.end = Floor(lvinfo.end / cellSize) * cellSize;
	lvinfo.cellSize = Vector3(cellSize);
	lvinfo.numCells = LightVolumeResolution;
	return lvinfo;
}