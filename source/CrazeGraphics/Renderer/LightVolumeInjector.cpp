#include "CrazeGraphicsPCH.h"
#include "LightVolumeInjector.h"

#include <memory>

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "Device.h"
#include "Graphics.h"

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

#include "PIXHelper.h"


using namespace Craze;
using namespace Craze::Graphics2;

bool LightVolumeInjector::initialize()
{
	m_fxFirstBounce.reset(new LVFirstBounceEffect());
	m_fxInjectRays.reset(new LVInjectRaysEffect());
	m_fxFirstBounce->initialize();
	m_fxInjectRays->initialize();

	m_RSMs[0] = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_COLOR_LINEAR, "RSM color");
	m_RSMs[1] = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_VECTOR4, "RSM normal");

	m_RSMDS = DepthStencil::Create2D(gpDevice, RSMResolution, RSMResolution, DEPTHSTENCIL_FORMAT_D24S8);

	m_dummy = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_8BIT_UNORM, "Dummy light volume render target");

	for (int i = 0; i < 3; ++i)
	{
		m_lightingVolumes[i] = RenderTarget::Create3D(gpDevice, LightVolumeResolution, LightVolumeResolution, LightVolumeResolution, 1, TEXTURE_FORMAT_HALFVECTOR4, "Light volume");
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
	
	c.SetOrthoProjection(1000.f, 1000.f, 10.f, 10000.f);
	return c;
}

Camera findSMCamera(const SpotLight& l, Scene* scene)
{
	return Camera();
}

std::shared_ptr<UAVBuffer> LightVolumeInjector::getCollidedRays()
{
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

		renderRSMs(scene, &c, viewProj);
		spawnRays(viewProj, scene->getCamera());

		traceRays();
		injectToLV(scene->getCamera());
	}
}

std::shared_ptr<RenderTarget>* LightVolumeInjector::getLightingVolumes(Scene* scene)
{
	PIXMARKER(L"Create lighting volumes");

	float black[] = { 0.f, 0.f, 0.f, 0.f };
	gpDevice->GetDeviceContext()->ClearRenderTargetView(m_lightingVolumes[0]->GetRenderTargetView(), black);
	gpDevice->GetDeviceContext()->ClearRenderTargetView(m_lightingVolumes[1]->GetRenderTargetView(), black);
	gpDevice->GetDeviceContext()->ClearRenderTargetView(m_lightingVolumes[2]->GetRenderTargetView(), black);

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
	
	return m_lightingVolumes;
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
	m_fxFirstBounce->doFirstBounce(m_dummy, m_RSMs, m_RSMDS, m_toTestRays, viewProj, cam);
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

	m_fxInjectRays->injectRays(m_collidedRays, m_lightingVolumes);
}

const LightVolumeInfo LightVolumeInjector::getLVInfo(const Camera* cam) const
{
	LightVolumeInfo lvinfo;
	
	float zSlice = 2000.f;
	
	lvinfo.start = cam->GetPosition();
	lvinfo.end = cam->GetPosition();

	Vector3 corners[8];
	cam->GetFrustumCorners(0.f, zSlice, corners);

	for (int i = 4; i < 8; ++i)
	{
		lvinfo.start = Min(lvinfo.start, corners[i]);
		lvinfo.end = Max(lvinfo.end, corners[i]);
	}
	const float gridFit = 500.f;
	lvinfo.start = Floor(lvinfo.start / gridFit) * gridFit;
	lvinfo.end = Ceil(lvinfo.end / gridFit) * gridFit;

	Vector3 delta = lvinfo.end - lvinfo.start;
	lvinfo.cellSize = delta / (float)LightVolumeResolution;

	lvinfo.numCells = LightVolumeResolution;
	return lvinfo;
}