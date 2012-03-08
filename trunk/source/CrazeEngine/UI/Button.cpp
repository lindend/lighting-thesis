#include "Button.h"
#include "Graphics/Geometry/GeometryData.h"
#include "EventLogger.h"

using Craze::Graphics::UI::Button;
using std::string;
using Craze::Math::Vector2;
using Craze::Math::Vector3;

Button::Button(Craze::Graphics::IDevice *pDevice, string name) : Image(pDevice, name)
{

	m_Params.clickable = true;
	m_Params.depth = 1;
	m_Params.dragable = false;
	m_Params.level = Craze::Graphics::UI::CL_MID;
	m_Params.visible = true;

}

Button::Button(Craze::Graphics::IDevice *pDevice, float xpos, float ypos, string name) : Image(pDevice, xpos, ypos, name)
{
	m_Params.clickable = true;
	m_Params.depth = 1;
	m_Params.dragable = false;
	m_Params.level = Craze::Graphics::UI::CL_MID;
	m_Params.visible = true;

}

Button::Button(Craze::Graphics::IDevice *pDevice, float xpos, float ypos, float width, float height, std::string name) : Image(pDevice, xpos, ypos, width, height, name)
{
	m_Params.clickable = true;
	m_Params.depth = 1;
	m_Params.dragable = false;
	m_Params.level = Craze::Graphics::UI::CL_MID;
	m_Params.visible = true;

}

void Button::SetClickCallback(Craze::Graphics::UI::ClickCallback callback)
{
	m_Callback = callback;
}

bool Button::VHandleClick()
{
	m_Callback(this);

	return true;
}