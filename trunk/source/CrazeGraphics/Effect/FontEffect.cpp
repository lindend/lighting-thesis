#include "CrazeGraphicsPCH.h"
#include "FontEffect.h"

#include "../Graphics.h"
#include "../EffectUtil/CBufferHelper.hpp"
#include "../Font/FontVertex.h"

using namespace Craze::Graphics2;
using namespace Craze;


void FontEffect::setObjectProperties(const Matrix4& world, const Material& material)
{
	CBPerObject data;
	data.world = world;

	ID3D11ShaderResourceView* srv = material.m_decal->GetResourceView();
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &srv);

	gpDevice->GetCbuffers()->SetObject(data);
}

const D3D11_INPUT_ELEMENT_DESC* FontEffect::getLayout(int& count)
{
	return FontVertex::GetLayoutDesc(count);
}