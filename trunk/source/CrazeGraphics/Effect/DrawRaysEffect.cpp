#include "CrazeGraphicsPCH.h"
#include "DrawRaysEffect.h"

using namespace Craze;
using namespace Craze::Graphics2;

const D3D11_INPUT_ELEMENT_DESC* DrawRaysEffect::getLayout(int& count)
{
	count = 2;

	static const D3D11_INPUT_ELEMENT_DESC pDesc[] = 
	{
		{"POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	return pDesc;
}