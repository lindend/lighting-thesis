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

#include "Effect/IEffect.h"
#include "EffectUtil/EffectHelper.h"
#include "EffectUtil/CBufferHelper.hpp"
#include "EffectUtil/ShaderResource.h"

#include "Model.h"
#include "Geometry/MeshData.h"
#include "GPUProfiler/GPUProfiler.h"

#include "PIXHelper.h"

using namespace Craze;
using namespace Craze::Graphics2;

const int LightVolumeInjector::LightVolumeResolution = 16;
const int LightVolumeInjector::MaxPhotonRays = 128 * 128 * 8;
const float LightVolumeInjector::MinDynamicity = 0.01f;

//VS_OUT main(float3 origin : ORIGIN, uint color : COLOR, float3 end : ENDPOINT, float dynamicity : DYNAMICITY)
const D3D11_INPUT_ELEMENT_DESC RayBufferElementDesc[] = 
{
	{"ORIGIN",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",      0, DXGI_FORMAT_R32_UINT,        0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"ENDPOINT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"DYNAMICITY", 0, DXGI_FORMAT_R32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

bool LightVolumeInjector::initialize()
{
    if (!initEffects())
    {
        return false;
    }

    //Initialize the lighting volumes
	for(int j = 0; j < 2; ++j)
	{
		for (int i = 0; i < CRAZE_NUM_LV; ++i)
		{
			m_lightVolumes[j][i] = RenderTarget::Create3D(gpDevice, LightVolumeResolution, LightVolumeResolution, LightVolumeResolution, 1, TEXTURE_FORMAT_HALFVECTOR4, "Light volume");
			float black[] = { 0.f, 0.f, 0.f, MinDynamicity};
			gpDevice->GetDeviceContext()->ClearRenderTargetView(m_lightVolumes[j][i]->GetRenderTargetView(), black);
		}
	}

    /*
    The random texture contains uniformly distributed directions on the hemisphere in the RGB channels. In the A channel is a
    random number.
    */
    m_random = std::dynamic_pointer_cast<const TextureResource>(gResMgr.loadResource(gFileDataLoader.addFile("random.dds")));

    //Initialize the buffers
	m_frustumInfoCBuffer = EffectHelper::CreateConstantBuffer(gpDevice, sizeof(Vector4) * 10);
    m_kdSceneInfoCBuffer = EffectHelper::CreateConstantBuffer(gpDevice, sizeof(Vector4) * 2);

	m_rayBuffer = UAVBuffer::Create(gpDevice, sizeof(float) * 8, MaxPhotonRays, true, "Ray buffer");
    m_tracedRays = UAVBuffer::Create(gpDevice, sizeof(float) * 8, MaxPhotonRays, true, "Traced rays");
	m_tessellatedRays = UAVBuffer::Create(gpDevice, sizeof(float) * 8, MaxPhotonRays, true, "Tessellated rays");

    m_rayVertices = GeometryBuffer::Create(gpDevice, GeometryBuffer::VERTEX, nullptr, sizeof(float) * 8, MaxPhotonRays, true, "Ray vertices");

   // m_rayCountBuffer = SRVBuffer::CreateRaw(gpDevice, DXGI_FORMAT_R32_UINT, 4, nullptr, "Ray count buffer");

    unsigned int args[] = { 0, 1, 0, 0};
	m_argBuffer = SRVBuffer::CreateRawArg(gpDevice, sizeof(unsigned int) * 4, args, "LVInjectRays arg buffer");


    if (!initGeometry())
    {
        return false;
    }

    /*
    Create the blend state to be used for when rasterizing the rays into the light volumes.
    This is a simple additive blend state, except that the alpha channel picks the maximum
    alpha value. The reason for the alpha channel doing so is that the light dynamicity is 
    stored in this value and we want the most dynamic light to set the value for the other.
    */
	CD3D11_BLEND_DESC bDesc;
	bDesc.IndependentBlendEnable = false;
	bDesc.AlphaToCoverageEnable = false;
	bDesc.RenderTarget[0].BlendEnable = true;
    bDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	bDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	
    bDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
	bDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	if (FAILED(gpDevice->GetDevice()->CreateBlendState(&bDesc, &m_addBS)))
    {
        return false;
    }

    /*
    The blend state for when merging the light volumes is using additive blending scaled with alpha.
    The resulting alpha should be the alpha used in the new light volume. That alpha value will in
    turn affect the light dynamicity in the next frame.
    */
	bDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;

    bDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    bDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	if (FAILED(gpDevice->GetDevice()->CreateBlendState(&bDesc, &m_mergeBS)))
    {
        return false;
    }

    /*
    The rasterizer state for when the rays are rendered into the light volume. Here,
    AntialiasedLineEnable is turned on to get a bit additional blur to the light.
    Whether or not this is beneficial to the quality of the lighting is something we
    probably should investigate more closely.
    UPDATE: Enabling antialiased lines produces some artifacts with the location
    of the lighting, so we have turned it off.
    */
	D3D11_RASTERIZER_DESC rsDesc;
	rsDesc.AntialiasedLineEnable = false;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.f;
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.FrontCounterClockwise = true;
	rsDesc.MultisampleEnable = false;
	rsDesc.ScissorEnable = false;
	rsDesc.SlopeScaledDepthBias = 0.f;
	if (FAILED(gpDevice->GetDevice()->CreateRasterizerState(&rsDesc, &m_AALinesRS)))
    {
        return false;
    }

	return true;
}

bool LightVolumeInjector::initEffects()
{
	m_fxInjectRays.reset(new IEffect(RayBufferElementDesc, 4));
	m_fxMergeLV.reset(new IEffect());
    m_firstBounceFx.reset(new IEffect());

    //Check if spherical harmonics or cube maps should be used for storing the light and select the appropriate shader.
	if (!m_fxInjectRays->initialize("RayTracing/InjectTessellated.vsh",
#ifdef CRAZE_USE_SH_LV
		"RayTracing/RasterizeSH.psh",
#else
		"RayTracing/RasterizeRaysCM.psh",
#endif
		"RayTracing/InjectTessellated.gsh"))
    {
        return false;
    }

	if (!m_fxMergeLV->initialize("RayTracing/MergeLVs.vsh", "RayTracing/MergeLVs.psh", "RayTracing/MergeLVs.gsh"))
    {
        return false;
    }

    if (!m_firstBounceFx->initialize("ScreenQuad.vsh", "RayTracing/FirstBounce.psh"))
    {
        return false;
    }

    m_rayTraceCS = EffectHelper::LoadShaderFromResource<ComputeShaderResource>("RayTracing/RayTrace.csh");
	m_tessellateCS = EffectHelper::LoadShaderFromResource<ComputeShaderResource>("RayTracing/TessellateRays.csh");

    if (!m_rayTraceCS || !m_tessellateCS)
    {
        return false;
    }

    return true;
}

bool LightVolumeInjector::initGeometry()
{
    //Hard coded collision geometry! 
	std::shared_ptr<const Model> triMesh = std::dynamic_pointer_cast<const Model>(gResMgr.loadResourceBlocking(gFileDataLoader.addFile("sponza/sponza_low.crm")));
	//std::shared_ptr<const Model> triMesh = std::dynamic_pointer_cast<const Model>(gResMgr.loadResourceBlocking(gFileDataLoader.addFile("content/terrainCollision.crm")));
    if (!triMesh)
    {
        return false;
    }

	std::tr1::shared_ptr<MeshData> meshData = triMesh->getMeshes()[0].mesh->getMeshData();
	const Vertex* verts = meshData->GetPosNormalUv();
	const unsigned short* indices = meshData->GetIndices();
	Vec3* tris = new Vec3[meshData->GetNumIndices()];
	for (int i = 0; i < meshData->GetNumIndices(); ++i)
	{
		tris[i] = verts[indices[i]].position;
	}
	setTriangles(tris, meshData->GetNumIndices() / 3);
	//delete [] tris;
    return true;
}

void LightVolumeInjector::setTriangles(const Vec3* tris, int numTris)
{
    m_geometryBB = CrNew BoundingBox();
    for (int i = 0; i < numTris * 3; ++i)
    {
        m_geometryBB->IncludePoint(tris[i]);
    }
    m_kdTree.Create((const DefaultTriangle*)tris, numTris, *m_geometryBB);

    unsigned int numKdNodes, numKdTris;
    const kdNode* nodes = m_kdTree.GetKdNodes(numKdNodes);
    const DefaultTriangle* triangles = m_kdTree.GetTriangles(numKdTris);

	m_triangleBuffer = SRVBuffer::CreateStructured(gpDevice, sizeof(DefaultTriangle), numKdTris, triangles, false, "LV Triangle buffer");
    m_kdTreeBuffer = SRVBuffer::CreateStructured(gpDevice, sizeof(kdNode), numKdNodes, nodes, false, "LV Kd nodes");
}

std::shared_ptr<UAVBuffer> LightVolumeInjector::getCollidedRays()
{
    return m_tessellatedRays;
}

void LightVolumeInjector::beginFrame()
{
    //Swap the active light volume and clear it to black
    float black[] = { 0.f, 0.f, 0.f, MinDynamicity };
	m_activeLightVolume = (m_activeLightVolume + 1) % 2;
	for (int i = 0; i < CRAZE_NUM_LV; ++i)
	{
		gpDevice->GetDeviceContext()->ClearRenderTargetView(m_lightVolumes[m_activeLightVolume][i]->GetRenderTargetView(), black);
	}
    m_numLights = 0;
}

void expand(const Vector3* vs, Vector3* vout, int i0, int i1, int i2, int i3, float len)
{
    //Calculate directions from the first point to the other
	Vector3 d0 = Normalize(vs[i1] - vs[i0]);
	Vector3 d1 = Normalize(vs[i2] - vs[i0]);
	Vector3 d2 = Normalize(vs[i3] - vs[i0]);
    
    //Move i0 away from every point and every point away from i0
	vout[i0] = (d0 + d1 + d2) * -len;
	vout[i1] = vout[i1] + d0 * len;
	vout[i2] = vout[i2] + d1 * len;
	vout[i3] = vout[i3] + d2 * len;
}

void LightVolumeInjector::addLight(const Vec3& color, float lightDynamicity, const Matrix4& lightViewProj, std::shared_ptr<RenderTarget> RSM[2], std::shared_ptr<DepthStencil> RSMdepth, Camera* cam)
{
    if (!m_random.get())
	{
		return;
	}

    GPU_PROFILE("Add light");

	Vector3 corners[8];
	cam->GetFrustumCorners(1.f, 2000.f, corners);

    /*
    Shift the frustum corners from the camera outward in order to include rays that are intersecting
    visible volumes, but that are not directly visible. These rays will still contribute to the lighting
    of the visible scene and artifacts will be introduced if they are discarded.
    */
	Vector3 cornerAdjustment[8];
	ZeroMemory(cornerAdjustment, sizeof(Vector3) * 8);
	float len = 60.f;
    //Move the corners away from each other.
	expand(corners, cornerAdjustment, 0, 1, 3, 4, len);
	expand(corners, cornerAdjustment, 2, 1, 3, 6, len);
	expand(corners, cornerAdjustment, 5, 1, 5, 6, len);
	expand(corners, cornerAdjustment, 7, 3, 4, 6, len);
	
    //Apply the expansion
	for (int i = 0; i < 8; ++i)
	{
		corners[i] = corners[i] + cornerAdjustment[i];
	}

    //Store the frustum information in the constant buffer
	CBufferHelper cb(gpDevice, m_frustumInfoCBuffer);
	for (int i = 0; i < 8; ++i)
	{
		cb[i] = corners[i].v;
	}
    cb[8] = color;
    cb[8].W() = lightDynamicity;
	cb.Unmap();
	
    {
        GPU_PROFILE("First bounce PS");
        m_firstBounceFx->set();

        gpDevice->GetDeviceContext()->PSSetConstantBuffers(1, 1, &m_frustumInfoCBuffer);

	    //gpDevice->GetDeviceContext()->CSSetShader(m_firstBounceCS->m_shader, nullptr, 0);

        //Send the inverse view x projection matrix of the light source to a constant buffer
	    CBPerLight cbLight;
	    cbLight.lightViewProj = lightViewProj.GetInverse();
	    gpDevice->GetCbuffers()->SetLight(cbLight);

        //Set the UAV, reset the counter if this is the first light
	    ID3D11RenderTargetView* rtv = nullptr;
	    ID3D11UnorderedAccessView* uav = m_rayBuffer->GetAppendConsumeUAV();
	    unsigned int initCount = m_numLights++ == 0 ? 0 : -1;

        D3D11_VIEWPORT vp;
        vp.Height = RSM[0]->GetHeight();
        vp.Width = RSM[0]->GetWidth();
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f;
        gpDevice->GetDeviceContext()->RSSetViewports(1, &vp);
	    gpDevice->GetDeviceContext()->OMSetRenderTargetsAndUnorderedAccessViews(1, &rtv, nullptr, 1, 1, &uav, &initCount);
	
	    ID3D11ShaderResourceView* srvs[] = { RSM[0]->GetResourceView(), RSM[1]->GetResourceView(), m_random.get()->GetResourceView(), RSMdepth->GetSRV() };
	    gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, srvs);
        gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        gpDevice->GetDeviceContext()->Draw(3, 0);
        //Each thread group is 16x16 in size at the moment.
        //gpDevice->GetDeviceContext()->Dispatch(RSM[0]->GetWidth() / 16, RSM[0]->GetHeight() / 16, 1);

	    ZeroMemory(srvs, sizeof(void*) * 4);
	    gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, srvs);
        gpDevice->SetRenderTarget(nullptr);
    }
}

std::shared_ptr<RenderTarget>* LightVolumeInjector::buildLightingVolumes()
{
	PIXMARKER(L"Build lighting volumes");
    //return m_lightVolumes[m_activeLightVolume];
	MEM_AUTO_MARK_STACK;

    int bvProf = gpGraphics->m_profiler->beginBlock("Build LV");

    int prof;
	prof = gpGraphics->m_profiler->beginBlock("Trace rays");
	traceRays();
	gpGraphics->m_profiler->endBlock(prof);

	prof = gpGraphics->m_profiler->beginBlock("Tessellate rays");
	tessellateRays();
	gpGraphics->m_profiler->endBlock(prof);

	prof = gpGraphics->m_profiler->beginBlock("Inject into LV");
	injectToLV();
	gpGraphics->m_profiler->endBlock(prof);
	
	prof = gpGraphics->m_profiler->beginBlock("Merge LVs");
	mergeToTarget();
	gpGraphics->m_profiler->endBlock(prof);

    gpGraphics->m_profiler->endBlock(bvProf);

	return m_lightVolumes[m_activeLightVolume];
}

void LightVolumeInjector::traceRays()
{
	PIXMARKER(L"Trace rays");

    //gpDevice->GetDeviceContext()->CopyStructureCount(m_rayCountBuffer->GetBuffer(), 0, m_rayBuffer->GetUAV());

    ID3D11UnorderedAccessView* UAV = m_tracedRays->GetAppendConsumeUAV();
	u32 initCount = 0;
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &UAV, &initCount);

    ID3D11ShaderResourceView* srvs[] = { m_triangleBuffer->GetSRV(), m_kdTreeBuffer->GetSRV(), m_rayBuffer->GetSRV() };
	gpDevice->GetDeviceContext()->CSSetShaderResources(0, 3, srvs);

    /*
    Send the cbuffer containing the bounding box of the scene to the constant buffer slot 1
    */
    CBufferHelper cb(gpDevice, m_kdSceneInfoCBuffer);
    cb[0] = m_geometryBB->m_Min.v;
    cb[1] = m_geometryBB->m_Max.v;
    cb.Unmap();
    gpDevice->GetDeviceContext()->CopyStructureCount(m_kdSceneInfoCBuffer, 12, m_rayBuffer->GetAppendConsumeUAV());

    gpDevice->GetDeviceContext()->CSSetConstantBuffers(1, 1, &m_kdSceneInfoCBuffer);

	gpDevice->GetDeviceContext()->CSSetShader(m_rayTraceCS->m_shader, nullptr, 0);
    /*
    Each thread group contains 128 threads, and we need one thread for each ray. Ideally,
    this should use DispatchIndirect with an argument buffer containing only the needed
    amount of threads. This a TODO and will require a new compute shader to output
    NumPhotons / 128 (or whatever the thread count it) into the buffer.
    */
	gpDevice->GetDeviceContext()->Dispatch(MaxPhotonRays / 128, 1, 1);

	UAV = nullptr;
    ZeroMemory(srvs, sizeof(void*) * 3);
    gpDevice->GetDeviceContext()->CSSetShaderResources(0, 3, srvs);
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &UAV, &initCount);
}
void LightVolumeInjector::tessellateRays()
{
	PIXMARKER(L"Tessellate rays");
	ID3D11UnorderedAccessView* UAVs[] = { m_tracedRays->GetUAV(), m_tessellatedRays->GetAppendConsumeUAV() };
	u32 initCounts[] = { -1, 0 };
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 2, UAVs, initCounts);
    
	gpDevice->GetDeviceContext()->CSSetShader(m_tessellateCS->m_shader, nullptr, 0);
    /*
    The same note as for the traceRays dispatch call applies here.
    */
	gpDevice->GetDeviceContext()->Dispatch(MaxPhotonRays / 128, 1, 1);

	ZeroMemory(UAVs, sizeof(void*) * 2);
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 2, UAVs, initCounts);
}

void LightVolumeInjector::copyRaysToVertexBuffer()
{
    gpDevice->GetDeviceContext()->CopyResource(m_rayVertices->GetBuffer(), m_tessellatedRays->GetBuffer());
}

void LightVolumeInjector::injectToLV()
{
	PIXMARKER(L"Inject rays to light volume");

    auto dc = gpDevice->GetDeviceContext();

	float bf[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	dc->OMSetBlendState(m_addBS, bf, 0xFFFFFFFF);
	dc->RSSetState(m_AALinesRS);
	m_fxInjectRays->set();

    {
        //GPU_PROFILE("Rasterize");
	    /*
        Prepare the argument buffer for the indirect call by copying the number of rays in the
        tessellated rays buffer to the vertex count per primitive field of the arg buffer. The
        number of instances field has already been set to 1.
        */
	    dc->CopyStructureCount(m_argBuffer->GetBuffer(), 0, m_tessellatedRays->GetAppendConsumeUAV());
    
        //Clear the vertex buffer, index buffer and input assembly (on a second thought, not sure if this is necessary).
	    ID3D11Buffer* vs = nullptr;//m_rayVertices->GetBuffer();
	    unsigned int stride = sizeof(float) * 8;
	    unsigned int offset = 0;
	    dc->IASetVertexBuffers(0, 1, &vs, &stride, &offset);
        dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);

	    ID3D11ShaderResourceView* srv = m_tessellatedRays->GetSRV();
	    dc->VSSetShaderResources(0, 1, &srv);
	    gpDevice->SetRenderTargets(m_lightVolumes[m_activeLightVolume], CRAZE_NUM_LV, nullptr);

	    /*
        Draw the rays using the arg buffer, the rays are expanded into lines in the geometry
        shader and we send them as points until then.
        */
	    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        dc->DrawInstancedIndirect(m_argBuffer->GetBuffer(), 0);

	    srv = nullptr;
	    dc->VSSetShaderResources(0, 1, &srv);

	    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
        m_fxInjectRays->reset();

	    dc->RSSetState(nullptr);

	    dc->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);
    }
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
		srvs[i] = m_lightVolumes[(m_activeLightVolume + 1) % 2][i]->GetResourceView();
	}
	
	gpDevice->SetRenderTargets(m_lightVolumes[m_activeLightVolume], CRAZE_NUM_LV, nullptr);
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, CRAZE_NUM_LV, srvs);
    //Draw a screen quad for each slice in the light volume
	gpDevice->GetDeviceContext()->DrawInstanced(3, LightVolumeResolution, 0, 0);
	m_fxMergeLV->reset();

	gpDevice->SetRenderTarget(nullptr);

	gpDevice->GetDeviceContext()->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);
}

const LightVolumeInfo LightVolumeInjector::getLVInfo(const Camera* cam) const
{
	LightVolumeInfo lvinfo;
	
    //Hard coded range of 2000 units.
	float zSlice = 2000.f;
	
	lvinfo.start = cam->GetPosition();
	lvinfo.end = cam->GetPosition();

	Vector3 corners[8];
	cam->GetFrustumCorners(2.f, zSlice, corners);
	
    /*
    Find the size of the light volume by finding the maximum distance between two
    corners in the frustum. Also keep track of the minimum coordinates of the corners
    in every channel to be able to determine where the light volume should start.
    */
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

    /*
    Snap the start position to a multiple of the cell size. Not doing so results in very
    visible artifacts when the light volume moves.
    */
	Vector3 cellStart = Floor(lvinfo.start / cellSize);
	Vector3 snapStart = cellStart * cellSize;
	lvinfo.start = snapStart;
	lvinfo.end = Floor(lvinfo.end / cellSize) * cellSize;
	lvinfo.cellSize = Vector3(cellSize);
	lvinfo.numCells = LightVolumeResolution;
	return lvinfo;
}

float Craze::Graphics2::calculateDynamicity(const Vector3& prevPos, const Vector3& prevDir, const Vector3& pos, const Vector3& dir)
{
    Vector3 deltaPos = pos - prevPos;
    
    const float minDynamicity = 0.005f;

    //This will have to be adjusted depending on the scene scale
    float posDynamicity = 1.f - pow(0.99f, Length(deltaPos));
    float rotDynamicity = 1.f - Dot(Normalize(dir), Normalize(prevDir));
    rotDynamicity = rotDynamicity * 5.f;

    //Clamp the dynamicity to less than or equal to 1
    float dynamicity = Min(posDynamicity + rotDynamicity, 1.f);

    return Max(dynamicity, minDynamicity);
}