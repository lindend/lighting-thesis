#include "CrazeGraphicsPCH.h"
#include "ShadowEffect.h"

#include "../Graphics.h"
#include "../Device.h"

using namespace Craze;
using namespace Craze::Graphics2;

void ShadowEffect::setObjectProperties(const Matrix4& world, const Material& material)
{
	CBPerObject data;
	data.world = world;

	gpDevice->GetCbuffers()->SetObject(data);
}