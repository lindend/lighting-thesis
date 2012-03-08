#pragma once

#include <D3D11.h>

#include "CrazeMath.h"

namespace Craze
{
	namespace Graphics2
	{
		struct FontVertex
		{
			Vector2 position;
			Vector2 size;
			Vector2 startUV;
			Vector2 endUV;

			static const D3D11_INPUT_ELEMENT_DESC* GetLayoutDesc(int& count)
			{
				count = 4;

				static const D3D11_INPUT_ELEMENT_DESC pDesc[] =  {
						{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
						{"POSITION", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
						{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
						{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
				};

				return pDesc;
			}
		};
	}
}