#include "CrazeGraphicsPCH.h"
#include "LightAccumulationEffect.h"
#include "../EffectUtil/CBufferHelper.hpp"
#include "../Graphics.h"

#include "../Texture/RenderTarget.h"

#include "../Light/SpotLight.h"
#include "../Light/PointLight.h"
#include "../Light/ProjectedLight.h"
#include "../Light/DirectionalLight.h"

using namespace Craze::Graphics2;
using namespace Craze;

LightAccumulationEffect::LightAccumulationEffect()
{
	m_vertexShader = 0;
	m_pixelShaderSpot = 0;

	m_textures[0] = NULL;
	m_textures[1] = NULL;
	m_textures[2] = NULL;

}

void LightAccumulationEffect::destroy()
{
	delete m_noise;
	m_noise = nullptr;
	SAFE_RELEASE(m_pixelShaderDir);
	SAFE_RELEASE(m_vertexShader);
	SAFE_RELEASE(m_pixelShaderSpot);
}

LightAccumulationEffect::~LightAccumulationEffect()
{
	destroy();
}

bool LightAccumulationEffect::initialize()
{

	if (m_vertexShader = EffectHelper::CompileVS(gpDevice, "Shaders/LightAccVS.vsh", &m_byteCode))
	{
		if (m_pixelShaderSpot = EffectHelper::CompilePS(gpDevice, "Shaders/LightAccPS_Spot.psh"))
		{
			if (m_pixelShaderDir = EffectHelper::CompilePS(gpDevice, "Shaders/LightAccPS_Dir.psh"))
			{
				if (m_noise = Texture::CreateFromFile(gpDevice, "Media/noise.png", false))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void LightAccumulationEffect::set(Texture* normSpec, Texture* depth, const std::vector<RenderTarget*>& LPV)
{
	auto resView = m_noise->GetResourceView();
	gpDevice->GetDeviceContext()->PSSetShaderResources(3, 1, &resView);

	gpDevice->SetShader(m_vertexShader);

	m_textures[0] = normSpec->GetResourceView();
	m_textures[1] = depth->GetResourceView();
	m_textures[2] = NULL;
	m_textures[3] = LPV[0]->GetResourceView();
	m_textures[4] = LPV[1]->GetResourceView();
	m_textures[5] = LPV[2]->GetResourceView();

	gpDevice->GetDeviceContext()->PSSetShaderResources(4, 6, m_textures);
	gpDevice->SetShader(m_pixelShaderSpot);

	gpDevice->SetCurrentEffect(this);

}

void LightAccumulationEffect::setLight(const SpotLight& light)
{
	gpDevice->SetShader(m_pixelShaderSpot);
	gpDevice->GetCbuffers()->SetLight(light);
}

void LightAccumulationEffect::setLight(const DirectionalLight& light)
{
	gpDevice->SetShader(m_pixelShaderDir);
	gpDevice->GetCbuffers()->SetLight(light);
}

void LightAccumulationEffect::setLight(const DirectionalLight& light, const Matrix4& viewProj, Vec3 lightPos, Texture* shadowMap)
{
	gpDevice->SetShader(m_pixelShaderDir);
	gpDevice->GetCbuffers()->SetLight(light, viewProj, lightPos);
	m_textures[2] = shadowMap ? shadowMap->GetResourceView() : 0;

	gpDevice->GetDeviceContext()->PSSetShaderResources(6, 1, &m_textures[2]);
}

void LightAccumulationEffect::reset()
{
	m_textures[0] = NULL;
	m_textures[1] = NULL;
	m_textures[2] = NULL;
	m_textures[3] = NULL;
	m_textures[4] = NULL;
	m_textures[5] = NULL;

	gpDevice->GetDeviceContext()->PSSetShaderResources(4, 6, m_textures);
}