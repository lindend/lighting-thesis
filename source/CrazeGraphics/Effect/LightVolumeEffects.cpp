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