#include "CrazeGraphicsPCH.h"
#include "DrawRaysEffect.h"

using namespace Craze;
using namespace Craze::Graphics2;

static const D3D11_INPUT_ELEMENT_DESC RayBufferVertexLayout[] = 
{
	{"POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

DrawRaysEffect::DrawRaysEffect() : IEffect(RayBufferVertexLayout, 2)
{
}