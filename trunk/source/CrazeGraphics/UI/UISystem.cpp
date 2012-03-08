#include "CrazeGraphicsPCH.h"
#include "UISystem.h"
#include "Resource/ResourceManager.h"

#include <map>

#include "gwen/Gwen.h"
#include "gwen/Skins/Simple.h"
#include "gwen/Controls/Canvas.h"
#include "gwen/Input/Windows.h"

#include "gwen/Controls/Button.h"
#include "gwen/Controls/CheckBox.h"
#include "gwen/Controls/ImagePanel.h"

#include "CrazeGwenRenderer.h"
#include "../Device.h"

using namespace Craze;
using namespace Craze::Graphics2;

CrazeGwenRenderer* pRenderer = nullptr;
Gwen::Skin::Simple* pSkin;
Gwen::Controls::Canvas* pCanvas = nullptr;
Gwen::Input::Windows inputHelper;

bool Craze::Graphics2::ui_init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	pRenderer = CrNew CrazeGwenRenderer();
	if (!pRenderer->Initialize())
	{
		return false;
	}

	pSkin = new Gwen::Skin::Simple();
	pSkin->SetRender(pRenderer);

	pCanvas = new Gwen::Controls::Canvas(pSkin);
	Vector2 vp = gpDevice->GetViewPort();
	pCanvas->SetSize((int)vp.x, (int)vp.y);

	inputHelper.Initialize(pCanvas);

	return true;
}

bool Craze::Graphics2::ui_handleInput(MSG message)
{
	return inputHelper.ProcessMessage(message);
}

void Craze::Graphics2::ui_render()
{
	pCanvas->RenderCanvas();
}

void Craze::Graphics2::ui_shutdown()
{
	delete pCanvas;
	pCanvas = nullptr;

	delete pSkin;
	pSkin = nullptr;

	if (pRenderer)
	{
		pRenderer->Shutdown();
		delete pRenderer;
		pRenderer = nullptr;
	}
}

Gwen::Controls::Canvas* Craze::Graphics2::ui_getCanvas()
{
	return pCanvas;
}