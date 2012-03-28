#include "CrazeGraphicsPCH.h"
#include "LightingEffect.h"

#include "Device.h"
#include "EffectUtil/CBufferManager.h"


using namespace Craze;
using namespace Craze::Graphics2;

void LightingEffect::doLighting(const DirectionalLight& l, const Matrix4& lightViewProj, std::shared_ptr<RenderTarget> shadowMap)
{
	CBPerLight cb;
	cb.lightDir = l.dir.operator Craze::Vector3();
 	//cb.lightDir = TransformVec(view, l.dir);
	cb.lightViewProj = lightViewProj;
	cb.lightType = DIRECTIONALLIGHT;
	cb.lightColor = l.color;
	gpDevice->GetCbuffers()->SetLight(cb);

	IEffect::set();

	auto srv = shadowMap ? shadowMap->GetResourceView() : nullptr;
	gpDevice->GetDeviceContext()->PSSetShaderResources(7, 1, &srv);

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gpDevice->GetDeviceContext()->Draw(3, 0);

	srv = nullptr;
	gpDevice->GetDeviceContext()->PSSetShaderResources(7, 1, &srv);
}