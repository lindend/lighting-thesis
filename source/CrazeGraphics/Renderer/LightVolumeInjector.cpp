#include "CrazeGraphicsPCH.h"
#include "LightVolumeInjector.h"

#include <memory>

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

using namespace Craze;
using namespace Craze::Graphics2;

bool LightVolumeInjector::initialize()
{
	m_RSMs[0] = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_COLOR_LINEAR, "RSM color");
	m_RSMs[1] = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_VECTOR4, "RSM normal");

	m_RSMDS = DepthStencil::Create2D(gpDevice, RSMResolution, RSMResolution, DEPTHSTENCIL_FORMAT_D24S8);

	m_dummy = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_8BIT_UNORM, "Dummy light volume render target");

	for (int i = 0; i < 3; ++i)
	{
		m_lightingVolumes[i] = RenderTarget::Create3D(gpDevice, LightVolumeResolution, LightVolumeResolution, LightVolumeResolution, 1, TEXTURE_FORMAT_VECTOR4, "Light volume");
	}

	m_toTestRays = UAVBuffer::Create(gpDevice, sizeof(float) * 4 * 3, MaxPhotonRays, true, "To test rays");
	m_collidedRays = UAVBuffer::Create(gpDevice, sizeof(float) * 4 * 4, MaxPhotonRays, true, "Collided rays");

	m_rayTraceCS = EffectHelper::LoadShaderFromResource<ComputeShaderResource>("RayTracing/RayTrace.csh");

	return true;
}

void LightVolumeInjector::setTriangles(Vector3* tris, int numTris)
{
	m_triangleBuffer = SRVBuffer::CreateStructured(gpDevice, sizeof(float) * 4 * 3, numTris, tris, false, "Triangle buffer");
	m_numTriangles = numTris;
}

Camera findSMCamera(const Light& l, Scene* scene)
{
	Camera c;
	c.SetPosition(Vector3(0, 3000, 0));
	c.SetDirection(l.dir);
	c.SetUp(Vector3::UP);
	c.SetProjection(3.14f / 2.f, 1, 10, 100000);
	return c;
}

std::shared_ptr<RenderTarget>* LightVolumeInjector::getLightingVolumes(Scene* scene)
{
	/*
	Algorithm overview:
	X 1. Render RSMs
	X 2. Inject the photons from the RSM into m_toTestRays
	X 3. Test all the rays in m_toTestRays against the triangles in m_triangleBuffer and store the result in m_collidedRays
	  4. Inject all the rays in m_collidedRays into the light volumes
	  5. Return the light volumes and rejoice! :D (maybe we should instead light the scene here directly or something...)
	*/

	//Create a fake light that we can use for testing, 
	Light dir = createDirectionalLight(-Vector3::ONE, Vector3::ONE);

	renderRSMs(scene, dir);
	injectRays();

	return m_lightingVolumes;
}

void LightVolumeInjector::renderRSMs(Scene* scene, const Light& l)
{
	Camera c = findSMCamera(l, scene);

	static DrawList shadowScene;
	shadowScene.clear();
	scene->buildDrawList(&shadowScene, c.GetView()*c.GetProjection());

	gpDevice->SetRenderTargets(m_RSMs, 2, m_RSMDS->GetDepthStencilView());
	gpDevice->GetDeviceContext()->ClearDepthStencilView(m_RSMDS->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);

	gFxGBuffer.set(&c);

	for (auto i = shadowScene.begin(); i != shadowScene.end(); ++i)
	{
		gFxGBuffer.setObjectProperties(*i->second.m_transform, *i->second.m_material);
		i->second.m_mesh->draw();
	}
}

void LightVolumeInjector::injectRays()
{
	gFxInjectRays.inject(m_dummy, m_RSMs, m_RSMDS, m_toTestRays);
}

void LightVolumeInjector::traceRays()
{
	ID3D11ShaderResourceView* triSrv = m_triangleBuffer->GetSRV();
	gpDevice->GetDeviceContext()->CSSetShaderResources(0, 1, &triSrv);

	ID3D11UnorderedAccessView* UAVs[] = { m_toTestRays->GetUAV(), m_collidedRays->GetUAV() };
	u32 initCounts[] = { -1, 0 };
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 2, UAVs, initCounts);

	gpDevice->GetDeviceContext()->CSSetShader(m_rayTraceCS->m_shader, nullptr, 0);
	gpDevice->GetDeviceContext()->Dispatch(MaxPhotonRays / (32 * 8), 1, 1);
}