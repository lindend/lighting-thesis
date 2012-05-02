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

				return FontVertexDesc;
			}

            static const D3D11_INPUT_ELEMENT_DESC FontVertexDesc[];
		};
	}
}