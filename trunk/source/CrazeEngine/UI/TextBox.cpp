#include "TextBox.h"

#include "Graphics/Assets/Material.h"
#include "Graphics/Texture/TextureId.h"
#include "Graphics/Device/IDevice.h"
#include "Math/Matrix4.h"
#include "Math/Vector2.h"
#include "Graphics/Text/FontManager.h"

using namespace Craze::Graphics::UI;
using namespace Craze::Graphics;

TextBox::TextBox(IDevice* pDevice, std::string name)
{
	TextBox(pDevice, 0.0f, 0.0f, name);
}

TextBox::TextBox(IDevice* pDevice, float xpos, float ypos, std::string name)
{
	TextBox(pDevice, xpos, ypos, 1.0f, 1.0f, name);
}

TextBox::TextBox(IDevice* pDevice, float xpos, float ypos, float width, float height, std::string name)
{
	m_pDevice = pDevice;

	SetPosition(xpos, ypos);
	SetSize(width, height);

	m_Name = name;

	m_Text.m_Text = "";

	m_FontProps.m_Face = "Arial";
	m_FontProps.m_Height = 14;
	m_FontProps.m_Italics = false;
	m_FontProps.m_Weight = Fonts::NORMAL;

	m_Text.m_pFont = Fonts::FontManager::GetFont(m_FontProps);

	m_Params.clickable = false;
	m_Params.dragable = false;
	m_Params.visible = true;
	m_Params.level = CL_MID;
	m_Params.depth = 1;

	m_Text.m_Color = Fonts::Color(255, 0, 0, 0);
	m_Text.m_Align = Fonts::VA_TOP | Fonts::HA_LEFT;
	m_Text.m_ClipMode = Fonts::CM_WORDWRAP | Fonts::CM_NOCLIP;

	m_Material.SetMaterialId(MATERIAL_UI);

}

void TextBox::SetText(const std::string& str)
{
	m_Text.m_Text = str;
}

const std::string& TextBox::GetText() const
{
	return m_Text.m_Text;
}

const Fonts::FontProperties& TextBox::GetFontProperties() const
{
	return m_FontProps;
}
void TextBox::SetFontProperties(const Fonts::FontProperties& fp)
{
	m_FontProps = fp;	
	m_Text.m_pFont = Fonts::FontManager::GetFont(m_FontProps);
}

std::string TextBox::VGetName()
{
	return m_Name;
}


ComponentParams* TextBox::VGetParams()
{
	return &m_Params;
}

bool TextBox::VHandleClick()
{
	return false;

}

void TextBox::VDraw(Renderer::Renderer* pRenderer)
{
	//This is the matrix for the current object
	Math::Matrix4 topMat = pRenderer->TopMatrix();

	Math::Vector2 res = m_pDevice->VGetResolution();

	m_Text.m_Size.x = topMat[0][0] * res.x;
	m_Text.m_Size.y = topMat[1][1] * res.y;

	m_Text.m_Position.x = ((topMat[3][0]) / 2 + 0.5f) * res.x;
	m_Text.m_Position.y = (1.0f - ((topMat[3][1]) / 2 + 0.5f)) * res.y;

	pRenderer->DrawObject(&m_Text, &m_Material, Craze::Graphics::Renderer::RENDERQUE_SCREENSPACE, (float)(m_Params.depth * (3 - m_Params.level)));

}

void TextBox::VUpdate(float delta)
{
	
}

bool TextBox::VIsOver(float xpos, float ypos)
{

	if (xpos < m_Position.x - m_Size.x || ypos < m_Position.y - m_Size.y)
	{
		return false;
	}

	if (xpos > m_Position.x + m_Size.x || ypos > m_Position.y + m_Size.y)
	{
		return false;
	}

	return true;
}