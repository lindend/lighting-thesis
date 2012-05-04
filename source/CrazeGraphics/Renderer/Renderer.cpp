#include "CrazeGraphicsPCH.h"
#include "Renderer.h"

#include <sstream>

#include "Intersection/Frustum.h"

#include "../Graphics.h"
#include "../Device.h"
#include "../DrawList.h"
#include "../Scene/Scene.h"
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
#include "Effect/LightVolumeEffects.h"
#include "Effect/DebugDrawEffect.h"
#include "Effect/CopyToBackBuffer.h"
#include "Buffer/Buffer.h"
#include "Renderer/DrawRays.h"
#include "GPUProfiler/GPUProfiler.h"

#include "PIXHelper.h"

using namespace Craze::Graphics2;
using namespace Craze;

namespace Craze
{
    namespace Graphics2
    {
        Renderer* gpRenderer = nullptr;
        bool gUseIndirectLighting = true;
        bool gUseDirectLighting = true;
        bool gUseConstantAmbient = false;
        bool gUseShadows = true;
        bool gDrawRays = false;

        bool gSaveScreenShot;
        std::string gScreenShotPath;
    }
}

Renderer::Renderer() : m_lightVolumeInjector(this)
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
    const int shadowMapRes = 1024;
    m_pShadowMap = RenderTarget::Create2D(gpDevice, shadowMapRes, shadowMapRes, 1, TEXTURE_FORMAT_FLOAT, "Shadow map");
    m_pShadowDS = DepthStencil::Create2D(gpDevice, shadowMapRes, shadowMapRes, DEPTHSTENCIL_FORMAT_D24S8);

    const int RSMResolution = 128;
    m_RSMs[0] = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_HALFVECTOR4, "RSM0");
    m_RSMs[1] = RenderTarget::Create2D(gpDevice, RSMResolution, RSMResolution, 1, TEXTURE_FORMAT_HALFVECTOR4, "RSM0");
    m_RSMDS = DepthStencil::Create2D(gpDevice, RSMResolution, RSMResolution, DEPTHSTENCIL_FORMAT_D24S8);

    m_pScreenQuad = Mesh::createScreenQuad(gpDevice);

    /*m_pSpotLight.reset(new SpotLight(pDevice));
    m_pSpotLight->SetDiffuse(Vector3::ONE);
    Vector3 pos = Vector3(2.0f, 2.0f, -2.0f) * 2.0f;
    m_pSpotLight->SetPosition(pos);
    pos.Normalize();
    m_pSpotLight->SetDirection(pos * -1.0f, Vector3::UP);
    m_pSpotLight->SetProjection(PI / 5.0f, 90.0f, 0.1f);
    m_pSpotLight->SetSpecular(0.5f);*/

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
    smDesc.MinLOD = 0.f;
    smDesc.MaxLOD = D3D11_FLOAT32_MAX;
    smDesc.MaxAnisotropy = 16;
    smDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    smDesc.MipLODBias = 0.f;
    gpDevice->GetDevice()->CreateSamplerState(&smDesc, &m_pSamplerState);
    gpDevice->GetDeviceContext()->PSSetSamplers(0, 1, &m_pSamplerState);
    gpDevice->GetDeviceContext()->VSSetSamplers(0, 1, &m_pSamplerState);
    gpDevice->GetDeviceContext()->GSSetSamplers(0, 1, &m_pSamplerState);

    smDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

    gpDevice->GetDevice()->CreateSamplerState(&smDesc, &m_pPointSampler);
    gpDevice->GetDeviceContext()->PSSetSamplers(3, 1, &m_pPointSampler);
    gpDevice->GetDeviceContext()->VSSetSamplers(3, 1, &m_pPointSampler);
    gpDevice->GetDeviceContext()->GSSetSamplers(3, 1, &m_pPointSampler);

    smDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    smDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    smDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

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

float seed = 0.0f;
void Renderer::InitFrame(Scene* pScene, CBPerFrame &cbuffer)
{
    const Camera* pCam = pScene->getCamera();
    Matrix4 viewProj = pCam->GetView() * pCam->GetProjection();

    cbuffer.cameraPos = pCam->GetPosition();
    cbuffer.viewProj = viewProj;
    cbuffer.ambientColor = pScene->AmbientLight;

    LightVolumeInfo lvInfo = m_lightVolumeInjector.getLVInfo(pCam);
    static Vec3 prevStart = lvInfo.start;
    cbuffer.LVStart = lvInfo.start;
    cbuffer.LVEnd = lvInfo.end;
    cbuffer.LVCellSize = Vector4(lvInfo.cellSize, lvInfo.numCells);
    cbuffer.OldLVStart = prevStart;

    seed += .0315957429834f;
    cbuffer.Seed = seed;
    prevStart = lvInfo.start;
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
    Camera c;
    c.SetPosition(l.pos);
    c.SetDirection(l.direction);
    c.SetUp(Vector3::UP);
    c.SetProjection(l.angle * 2.f, 1.f, 1.f, 100000.f);

    return c;
}


void Renderer::RenderScene(Craze::Graphics2::Scene* pScene)
{
    PROFILEF();

    static DrawList mainScene;
    static DrawList shadowScene;
    mainScene.clear();
    shadowScene.clear();

    pScene->update();

    //If we should redraw the UI, begin creating a deferred device context with the rendering for it now

    const Camera* pCam = pScene->getCamera();
    Matrix4 viewProj = pCam->GetView() * pCam->GetProjection();

    int numDirLights;
    const DirectionalLight* dirLights = pScene->getDirectionalLights(numDirLights);

    std::shared_ptr<RenderTarget>* lightVolumes = buildLightVolumes(pScene, dirLights, numDirLights);

    //Thread this...
    pScene->buildDrawList(&mainScene, viewProj);

    float color[] = {0.5f, 0.5f, 0.5f, 0.f};
    gpDevice->GetDeviceContext()->ClearRenderTargetView(m_GBuffers[0]->GetRenderTargetView(), color);
    gpDevice->Clear(Vector4(0.5f, 0.5f, 0.5f, 1.0f));

    gpDevice->SetRenderTargets(m_GBuffers, NumGBuffers, gpDevice->GetDefaultDepthBuffer());
    gpDevice->ClearCache();

    int gbProf = gpGraphics->m_profiler->beginBlock("G-buffers");
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
    gpGraphics->m_profiler->endBlock(gbProf);


    //The gbuffers are ready, perform lighting
    ID3D11ShaderResourceView* pSRVs[4] = { m_GBuffers[0]->GetResourceView(), m_GBuffers[1]->GetResourceView(), m_GBuffers[2]->GetResourceView(), gpDevice->GetDefaultDepthSRV() };
    //gFxCSLighting.run(pCam, pSRVs, m_pOutputTarget->GetUAV(), visibleLights);

    //Do the other lights, with shadows
    float black[] = { 0.f, 0.f, 0.f, 0.f };
    gpDevice->GetDeviceContext()->ClearRenderTargetView(m_pOutputTarget->GetRenderTargetView(), black);

    const float bf[4] = {1.f, 1.f, 1.f, 1.f};

    ID3D11ShaderResourceView* pOutSRVs[] = { nullptr, nullptr };
    gpDevice->GetDeviceContext()->PSSetShaderResources(4, 2, pOutSRVs);

    int lightProf = gpGraphics->m_profiler->beginBlock("Direct lighting");
    if (gUseDirectLighting)
    {
        PIXMARKER(L"Do lighting");
        for (int i = 0; i < numDirLights; ++i)
        {
            const DirectionalLight& light = dirLights[i];
            Matrix4 lightViewProj = Matrix4::CreateView(light.dir * -2000.f, Vector3::ZERO, Vector3::UP) * Matrix4::CreateOrtho(2000.f, 2000.f, 1.f, 10000.f);

            if (gUseShadows)
            {
                PIXMARKER(L"Render shadow map");			
                shadowScene.clear();
                pScene->buildDrawList(&shadowScene, lightViewProj);

                gpDevice->GetDeviceContext()->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);

                gpDevice->GetDeviceContext()->ClearRenderTargetView(m_pShadowMap->GetRenderTargetView(), bf);
                gpDevice->GetDeviceContext()->ClearDepthStencilView(m_pShadowDS->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);
                gpDevice->SetRenderTarget(m_pShadowMap, m_pShadowDS);

                gpDevice->GetCbuffers()->SetLight(light, lightViewProj, light.dir * -1000.f);

                gFxShadow.set();

                for (auto i = shadowScene.begin(); i != shadowScene.end(); ++i)
                {
                    gFxShadow.setObjectProperties(*i->second.m_transform, *i->second.m_material);
                    i->second.m_mesh->draw();
                }
            }

            gpDevice->SetRenderTarget(m_pOutputTarget, nullptr);
            gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, pSRVs);

            gpDevice->GetDeviceContext()->OMSetBlendState(m_pLightBS, bf, 0xFFFFFFFF);
            gFxLighting.doLighting(light, lightViewProj, gUseShadows ? m_pShadowMap : nullptr);
        }

        drawLights(pScene->getVisibleSpotLights(viewProj), pScene);
    }
    gpGraphics->m_profiler->endBlock(lightProf);

    int useIIprof = gpGraphics->m_profiler->beginBlock("Add indirect lighting");
    if (gUseIndirectLighting)
    {
        PIXMARKER(L"Do indirect lighting");
        gpDevice->SetRenderTarget(m_pOutputTarget, nullptr);
        gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, pSRVs);
        gFxLVAmbientLighting.doLighting(lightVolumes, m_GBuffers, gpDevice->GetDefaultDepthSRV(), m_lightVolumeInjector.getLVInfo(pCam));
    }
    gpGraphics->m_profiler->endBlock(useIIprof);

    pSRVs[0] = nullptr;
    pSRVs[1] = nullptr;
    pSRVs[2] = nullptr;
    pSRVs[3] = nullptr;
    gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, pSRVs);
    gpDevice->GetDeviceContext()->PSSetShaderResources(4, 2, pSRVs);

    gpDevice->GetDeviceContext()->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);

    gpDevice->SetRenderTargets(&m_pOutputTarget, 1, gpDevice->GetDefaultDepthBuffer());

    {
        if (gDrawRays)
        {
            PIXMARKER(L"Draw rays");
            m_rayDrawer->render(m_lightVolumeInjector.getCollidedRays(), viewProj);
        }
    }

    gFxCopyToBack.doCopy(m_pOutputTarget);

    gpDevice->GetDeviceContext()->OMSetDepthStencilState(0, 0);

    if (gSaveScreenShot)
    {
        ID3D11Resource* backbufferRes;
        gpDevice->getBackBufferRTV()->GetResource(&backbufferRes);

        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.ArraySize = 1;
        texDesc.BindFlags = 0;
        texDesc.CPUAccessFlags = 0;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Vector2 res = gpDevice->GetViewPort();
        texDesc.Width = (int)res.x; 
        texDesc.Height = (int)res.y;
        texDesc.MipLevels = 1;
        texDesc.MiscFlags = 0;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;

        ID3D11Texture2D* texture;
        gpDevice->GetDevice()->CreateTexture2D(&texDesc, nullptr, &texture);
        gpDevice->GetDeviceContext()->CopyResource(texture, backbufferRes);

        D3DX11SaveTextureToFileA(gpDevice->GetDeviceContext(), texture, D3DX11_IFF_PNG, gScreenShotPath.c_str());
        texture->Release();
        backbufferRes->Release();

        gSaveScreenShot = false;
    }

}

void Renderer::drawLights(SpotLightArray spotLights, Scene* scene)
{
    float bf[] = {1.f, 1.f, 1.f, 1.f};
    for (int i = 0; i < spotLights.numLights; ++i)
    {
        const SpotLight& light = spotLights.spotLights[i];
        Matrix4 lightViewProj = Matrix4::CreateView(light.pos, light.pos + light.direction, Vector3::UP) * Matrix4::CreatePerspectiveFov(light.angle * 2.f, 1.f, 3.f, light.range);

        if (gUseShadows)
        {
            static DrawList shadowScene;
            PIXMARKER(L"Render shadow map");			
            shadowScene.clear();
            scene->buildDrawList(&shadowScene, lightViewProj);

            gpDevice->GetDeviceContext()->OMSetBlendState(nullptr, bf, 0xFFFFFFFF);

            gpDevice->GetDeviceContext()->ClearRenderTargetView(m_pShadowMap->GetRenderTargetView(), bf);
            gpDevice->GetDeviceContext()->ClearDepthStencilView(m_pShadowDS->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);
            gpDevice->SetRenderTarget(m_pShadowMap, m_pShadowDS);

            gpDevice->GetCbuffers()->SetLight(light);

            gFxShadow.set();

            for (auto i = shadowScene.begin(); i != shadowScene.end(); ++i)
            {
                gFxShadow.setObjectProperties(*i->second.m_transform, *i->second.m_material);
                i->second.m_mesh->draw();
            }
        }

        ID3D11ShaderResourceView* pSRVs[4] = { m_GBuffers[0]->GetResourceView(), m_GBuffers[1]->GetResourceView(), m_GBuffers[2]->GetResourceView(), gpDevice->GetDefaultDepthSRV() };
        gpDevice->SetRenderTarget(m_pOutputTarget, nullptr);
        gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, pSRVs);

        const float bf[4] = {1.f, 1.f, 1.f, 1.f};
        gpDevice->GetDeviceContext()->OMSetBlendState(m_pLightBS, bf, 0xFFFFFFFF);
        gFxLighting.doLighting(light, lightViewProj, gUseShadows ? m_pShadowMap : nullptr);
    }
}

std::shared_ptr<RenderTarget>* Renderer::buildLightVolumes(Scene* scene, const DirectionalLight* dirLights, int numDirLights)
{
    std::shared_ptr<RenderTarget>* lightVolumes = nullptr;
    DrawList shadowScene;

    PIXMARKER(L"Build lighting volumes");
    int generateLightVolumeProfiling = gpGraphics->m_profiler->beginBlock("Create lighting volumes");
    if (gUseIndirectLighting)
    {
        m_lightVolumeInjector.beginFrame();
        for (int i = 0; i < numDirLights; ++i)
        {
            const DirectionalLight& light = dirLights[i];
            Camera dlCam = findSMCamera(light, scene);
            Matrix4 lightViewProj = dlCam.GetView() * dlCam.GetProjection();//Matrix4::CreateView(light.dir * -2000.f, Vector3::ZERO, Vector3::UP) * Matrix4::CreateOrtho(2000.f, 2000.f, 1.f, 10000.f);

            {
                PIXMARKER(L"Create RSM");
                shadowScene.clear();
                scene->buildDrawList(&shadowScene, lightViewProj);

                gpDevice->SetRenderTargets(m_RSMs, 2, m_RSMDS->GetDepthStencilView());
                gpDevice->GetDeviceContext()->ClearDepthStencilView(m_RSMDS->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);

                gFxGBuffer.set(&dlCam);

                for (auto i = shadowScene.begin(); i != shadowScene.end(); ++i)
                {
                    gFxGBuffer.setObjectProperties(*i->second.m_transform, *i->second.m_material);
                    i->second.m_mesh->draw();
                }
            }
            gpDevice->SetRenderTarget(nullptr, nullptr);
            m_lightVolumeInjector.addLight(light.color, 0.02f, lightViewProj, m_RSMs, m_RSMDS, scene->getCamera());
        }

        //No visibility test is active, so IDENTITY is usable
        const SpotLightArray spotlights = scene->getVisibleSpotLights(Matrix4::IDENTITY);
        for (int i = 0; i < spotlights.numLights; ++i)
        {
            const SpotLight& light = spotlights.spotLights[i];
            Camera dlCam = findSMCamera(light, scene);
            Matrix4 lightViewProj = dlCam.GetView() * dlCam.GetProjection();//Matrix4::CreateView(light.dir * -2000.f, Vector3::ZERO, Vector3::UP) * Matrix4::CreateOrtho(2000.f, 2000.f, 1.f, 10000.f);

            {
                PIXMARKER(L"Create RSM");
                shadowScene.clear();
                scene->buildDrawList(&shadowScene, lightViewProj);

                gpDevice->SetRenderTargets(m_RSMs, 2, m_RSMDS->GetDepthStencilView());
                gpDevice->GetDeviceContext()->ClearDepthStencilView(m_RSMDS->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);

                gFxGBuffer.set(&dlCam);

                for (auto i = shadowScene.begin(); i != shadowScene.end(); ++i)
                {
                    gFxGBuffer.setObjectProperties(*i->second.m_transform, *i->second.m_material);
                    i->second.m_mesh->draw();
                }
            }
            gpDevice->SetRenderTarget(nullptr, nullptr);
            m_lightVolumeInjector.addLight(light.color, 0.2f, lightViewProj, m_RSMs, m_RSMDS, scene->getCamera());

        }


        lightVolumes = m_lightVolumeInjector.buildLightingVolumes();
    }
    gpGraphics->m_profiler->endBlock(generateLightVolumeProfiling);

    return lightVolumes;
}

void Renderer::BindScene(Scene* pScene)
{
    //m_ISPMRenderer.BindScene(pScene);
}

void Renderer::RenderLight(DirectionalLight* pLight, Scene* pScene)
{

}
