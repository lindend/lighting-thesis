#include "CrazeGraphicsPCH.h"
#include "LightingEffect.h"

#include "Device.h"
#include "EffectUtil/CBufferManager.h"


using namespace Craze;
using namespace Craze::Graphics2;

bool LightingEffect::initialize()
{
	m_spotLight = EffectHelper::LoadShaderFromResource<PixelShaderResource>("SpotLight.psh");
	return IEffect::initialize("ScreenQuad.vsh", "DirLight.psh");
}

void LightingEffect::doLighting(const DirectionalLight& l, const Matrix4& lightViewProj, std::shared_ptr<RenderTarget> shadowMap)
{
	CBPerLight cb;
	cb.lightDir = l.dir.operator Craze::Vector3();
 	//cb.lightDir = TransformVec(view, l.dir);
	cb.lightViewProj = lightViewProj;
	cb.lightColor = l.color;
	gpDevice->GetCbuffers()->SetLight(cb);

	IEffect::set();

	auto srv = shadowMap ? shadowMap->GetResourceView() : nullptr;
	gpDevice->GetDeviceContext()->PSSetShaderResources(7, 1, &srv);

	gpDevice->GetDeviceContext()->Draw(3, 0);

	srv = nullptr;
	gpDevice->GetDeviceContext()->PSSetShaderResources(7, 1, &srv);
}

void LightingEffect::doLighting(const SpotLight& l, const Matrix4& lightViewProj, std::shared_ptr<RenderTarget> shadowMap)
{
	CBPerLight cb;
	cb.lightDir = l.direction.operator Craze::Vector3();
	cb.lightViewProj = lightViewProj;
	cb.lightColor = l.color;
	cb.lightPos = l.pos;
	cb.lightAngle = l.angle;
	cb.lightRange = l.range;

	gpDevice->GetCbuffers()->SetLight(cb);

	gpDevice->SetShader(m_vs->m_shader);
	gpDevice->SetShader(m_spotLight->m_shader);

	auto srv = shadowMap ? shadowMap->GetResourceView() : nullptr;
	gpDevice->GetDeviceContext()->PSSetShaderResources(7, 1, &srv);

	gpDevice->GetDeviceContext()->Draw(3, 0);

	srv = nullptr;
	gpDevice->GetDeviceContext()->PSSetShaderResources(7, 1, &srv);
}