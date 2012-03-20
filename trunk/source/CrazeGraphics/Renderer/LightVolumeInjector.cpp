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

Camera findSMCamera(const Light& l, Scene* scene)
{
	Camera c;
	c.SetPosition(Vector3(0, 1000, 0));
	c.SetDirection(l.dir);
	if (Dot(c.GetDirection(), Vector3::UP) > 0.9f)
	{
		c.SetUp(Vector3::RIGHT);
	} else
	{
		c.SetUp(Vector3::UP);
	}
	
	c.SetProjection(3.14f / 2.f, 1, 10, 100000);
	return c;
}

std::shared_ptr<UAVBuffer> LightVolumeInjector::getCollidedRays()
{
	return m_collidedRays;
}

std::shared_ptr<RenderTarget>* LightVolumeInjector::getLightingVolumes(Scene* scene)
{
	/*
	Algorithm overview:
	X 1. Render RSMs
	X 2. Spawn the photons from the RSM into m_toTestRays
	X 3. Test all the rays in m_toTestRays against the triangles in m_triangleBuffer and store the result in m_collidedRays
	X 4. Inject all the rays in m_collidedRays into the light volumes
	  5. Return the light volumes and rejoice! :D (maybe we should instead light the scene here directly or something...)
	*/

	//Create a fake light that we can use for testing, 
	Light dir = createDirectionalLight(-Vector3::ONE, Vector3::ONE);

	PIXMARKER(L"Create lighting volumes");

	Camera c = findSMCamera(dir, scene);
	Matrix4 viewProj = c.GetView()*c.GetProjection();

	renderRSMs(scene, &c, viewProj);
	spawnRays(viewProj);
	traceRays();
	injectToLV(scene->getCamera());


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

void LightVolumeInjector::spawnRays(const Matrix4& viewProj)
{
	PIXMARKER(L"Spawn rays");
	m_fxFirstBounce->doFirstBounce(m_dummy, m_RSMs, m_RSMDS, m_toTestRays, viewProj);
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

	m_fxInjectRays->injectRays(m_collidedRays, m_lightingVolumes, lvinfo);
}

const LightVolumeInfo LightVolumeInjector::getLVInfo(const Camera* cam) const
{
	LightVolumeInfo lvinfo;
	
	float zSlice =  2000.f;
	Vector3 slicePt = cam->GetPosition() + cam->GetDirection() * zSlice;
	float upVecScale = Tan(cam->GetFovY() * 0.5f) * zSlice;
	Vector3 rightVec = Cross(cam->GetDirection(), cam->GetUp()) * upVecScale * cam->GetAspect();
	Vector3 upVec = cam->GetUp() * upVecScale;

	Vector3 corners[] = {
		slicePt + rightVec + upVec,
		slicePt + rightVec - upVec,
		slicePt - rightVec + upVec,
		slicePt - rightVec - upVec
	};

	lvinfo.start = cam->GetPosition();
	lvinfo.end = cam->GetPosition();

	for (int i = 0; i < 4; ++i)
	{
		lvinfo.start = Min(lvinfo.start, corners[i]);
		lvinfo.end = Max(lvinfo.end, corners[i]);
	}
	Vector3 delta = lvinfo.end - lvinfo.start;
	lvinfo.cellSize = delta / (float)LightVolumeResolution;
	Vector3 start = lvinfo.start;
	start = start / lvinfo.cellSize;
	start = Vector3((int)start->x, (int)start->y, (int)start->z);
	lvinfo.start = start  * lvinfo.cellSize;
	lvinfo.numCells = LightVolumeResolution;
	return lvinfo;
}