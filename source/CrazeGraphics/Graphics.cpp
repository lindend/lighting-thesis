#include "CrazeGraphicsPCH.h"
#include "Graphics.h"

#include "IL/il.h"
#include "IL/ilu.h"

#include "EventLogger.h"
#include "Resource/FileDataLoader.h"
#include "Util/CrazeHash.h"

#include "Model.h"
#include "Scene/Camera.h"
#include "Geometry/TerrainChunk.h"
#include "Font/FontManager.h"
#include "Renderer/Renderer.h"
#include "Effect/CopyToBackBuffer.h"
#include "Effect/GBufferEffect.h"
#include "Effect/LightAccumulationEffect.h"
#include "Effect/ShadingEffect.h"
#include "Effect/FontEffect.h"
#include "Effect/ShadowEffect.h"
#include "Effect/AmbientLighting.h"
#include "Effect/CSLighting.h"
#include "Effect/LightingEffect.h"
#include "Effect/PosMapEffect.h"
#include "Effect/NullEffect.h"
#include "Effect/DebugDrawEffect.h"
#include "Effect/LightVolumeEffects.h"

#include "EffectUtil/ShaderResourceHandler.h"
#include "UI/UISystem.h"

using namespace Craze;
using namespace Craze::Graphics2;

ModelResourceHandler gModelResHandler;
TextureResourceHandler gTexResHandler;

VertexShaderResourceHandler gVSResHandler;
PixelShaderResourceHandler gPSResHandler;
GeometryShaderResourceHandler gGSResHandler;
ComputeShaderResourceHandler gCSResHandler;
HullShaderResourceHandler gHSResHandler;
DomainShaderResourceHandler gDSResHandler;
TessShaderResourceHandler gTessResHandler;

namespace Craze
{
	namespace Graphics2
	{
		Graphics* gpGraphics;
		Resources* gpGfxResources;

		CopyToBackBuffer gFxCopyToBack;

		FontEffect gFxFont;
		GBufferEffect gFxGBuffer;
		LightAccumulationEffect gFxLightAcc;
		ShadingEffect gFxShading;
		ShadowEffect gFxShadow;
		AmbientLighting gFxAmbientLighting;
		CSLighting gFxCSLighting;
		LightingEffect gFxLighting;
		PosMapEffect gFxPosMap;
		NullEffect gFxNull;
		DebugDrawEffect gFxDebugDraw;
		LVAmbientLightingEffect gFxLVAmbientLighting;
	}
}

Graphics::Graphics()
{
	m_pScene = nullptr;
}

Graphics::~Graphics()
{

}

bool InitEffects()
{
	return
	gFxCopyToBack.initialize() &&
	gFxFont.initialize() &&
	gFxGBuffer.initialize() &&

	//gFxLightAcc.initialize() &&
	//gFxShading.initialize() &&

	gFxShadow.initialize() &&
	gFxAmbientLighting.initialize() &&
	gFxCSLighting.initialize() &&
	gFxLighting.initialize() &&
	gFxPosMap.initialize() &&
	gFxNull.initialize() &&
	gFxDebugDraw.initialize() &&
	gFxLVAmbientLighting.initialize() &&
	true;
}

void DestroyEffects()
{
	gFxCopyToBack.destroy();
	gFxFont.destroy();
	gFxGBuffer.destroy();
	gFxLightAcc.destroy();
	gFxShading.destroy();
	gFxShadow.destroy();
	gFxAmbientLighting.destroy();
	gFxCSLighting.destroy();
	gFxLighting.destroy();
	gFxPosMap.destroy();
	gFxNull.destroy();
	gFxDebugDraw.destroy();
	gFxLVAmbientLighting.destroy();

}

bool Craze::Graphics2::InitGraphics(HWND hWnd, unsigned int width, unsigned int height)
{
	gpGraphics = new Graphics();

	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		LOG_ERROR("Incorrect version of DevIL.dll found");
		return false;
	}

	ilInit();
	iluInit();

	if (!InitGraphicsDevice(width, height, false, hWnd))
	{
		LOG_ERROR("Unable to initialize the graphics engine");
		return false;
	}

	if (gpDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
	{
		LOG_CRITICAL("A graphics card capable of DirectX 11 is required");
		return false;
	}

#ifdef _DEBUG
	gFileDataLoader.addLocation("../source/CrazeGraphics/Shaders/");
#else
	gFileDataLoader.addLocation("Shaders/");
#endif

	gResMgr.setResourceHandling(hash32(".crm"), &gModelResHandler);
	gResMgr.setResourceHandling(hash32(".png"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".bmp"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".dds"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".exr"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".hdr"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".gif"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".jpg"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".psd"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".raw"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".tga"), &gTexResHandler);
	gResMgr.setResourceHandling(hash32(".tif"), &gTexResHandler);

	gResMgr.setResourceHandling(hash32(".vsh"), &gVSResHandler);
	gResMgr.setResourceHandling(hash32(".psh"), &gPSResHandler);
	gResMgr.setResourceHandling(hash32(".gsh"), &gGSResHandler);
	gResMgr.setResourceHandling(hash32(".csh"), &gCSResHandler);
	gResMgr.setResourceHandling(hash32(".hsh"), &gHSResHandler);
	gResMgr.setResourceHandling(hash32(".dsh"), &gDSResHandler);
	gResMgr.setResourceHandling(hash32(".tess"), &gTessResHandler);


	if (!InitEffects())
	{
		LOG_ERROR("Unable to initialize effects");
		return false;
	}

	if (!ui_init(gpDevice->GetDevice(), gpDevice->GetDeviceContext()))
	{
		LOG_ERROR("Unable to initialize UI system");
		return false;
	}

	InitFont();

	gpRenderer = CrNew Renderer();
	gpRenderer->Initialize();

	return true;
}

void Graphics::BindScene(Scene* pScene)
{
	gpRenderer->BindScene(pScene);
	m_pScene = pScene;
}

void Graphics::Render()
{
	PROFILEF();

	CBPerFrame cbuffer;

	Vector2 res = gpDevice->GetViewPort();
	cbuffer.invResolution = Vector2(1.0f / res.x, 1.0f / res.y);
	cbuffer.DisableAO = gpGraphics->Params.DisableAO;
	cbuffer.DisableIndirect = true;//gpGraphics->Params.DisableIndirect;
	cbuffer.DisableDirect = gpGraphics->Params.DisableDirect;
	cbuffer.BoostIndirect = gpGraphics->Params.BoostIndirect;
	cbuffer.UseESM = gpGraphics->Params.UseESM;

	if (m_pScene)
	{
		gpRenderer->InitFrame(m_pScene, cbuffer);
	}

	gpDevice->GetCbuffers()->SetFrame(cbuffer);

	gpDevice->Clear(Vector4(0.f, 0.f, 0.f, 0.f));

	if (m_pScene)
	{
		gpRenderer->RenderScene(m_pScene);
	}

	ui_render();

	gpDevice->ResetCounters();

	gpDevice->Swap();
}

void Craze::Graphics2::ShutdownGraphics()
{

	if (gpGraphics)
	{
		if (gpRenderer)
		{
			gpRenderer->Shutdown();
			delete gpRenderer;
			gpRenderer = 0;
		}

		ui_shutdown();

		gFontMgr.shutdown();

		DestroyEffects();

		ShutdownGraphicsDevice();

		delete gpGraphics;
		gpGraphics = 0;
	}
}
