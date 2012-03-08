#pragma once
#include <string>

#include "D3D11.h"

#include "Gwen/Controls/Canvas.h"
namespace Craze
{
	namespace Graphics2
	{
		bool ui_init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
		void ui_render();
		bool ui_handleInput(MSG message);
		void ui_shutdown();

		Gwen::Controls::Canvas* ui_getCanvas();
	}
}