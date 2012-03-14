#include "CrazeGraphicsPCH.h"
#include "LightingEffect.h"

#include "Device.h"
#include "EffectUtil/CBufferManager.h"


using namespace Craze;
using namespace Craze::Graphics2;

void LightingEffect::doLighting(const Light& l, const Matrix4& view, std::shared_ptr<RenderTarget> shadowMap)
{
	assert(l.type == DIRECTIONALLIGHT);

	CBPerLight cb;
	cb.lightDir = l.dir.operator Craze::Vector3();
 	//cb.lightDir = TransformVec(view, l.dir);

	cb.lightType = DIRECTIONALLIGHT;
	cb.lightColor = l.color.operator Craze::Vector3();
	gpDevice->GetCbuffers()->SetLight(cb);

	IEffect::set();

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gpDevice->GetDeviceContext()->Draw(3, 0);
}