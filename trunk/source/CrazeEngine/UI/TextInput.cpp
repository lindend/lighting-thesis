#include "TextInput.h"
#include "UserInterface.h"

using namespace Craze::Graphics;
using namespace Craze::Graphics::UI;

TextInput::TextInput(IDevice* pDevice, class UserInterface* pUI, std::string name) : TextBox(pDevice, name)
{
	m_pUI = pUI;
	m_Params.clickable = true;
	m_Active = false;
	m_BlinkTimer = 0.0f;
}

TextInput::TextInput(IDevice* pDevice, UserInterface* pUI, float xpos, float ypos, std::string name) : TextBox(pDevice, xpos, ypos, name)
{
	m_pUI = pUI;
	m_Params.clickable = true;
	m_Active = false;
	m_BlinkTimer = 0.0f;
}

TextInput::TextInput(IDevice* pDevice, UserInterface* pUI, float xpos, float ypos, float width, float height, std::string name) : TextBox(pDevice, xpos, ypos, width, height, name)
{
	m_pUI = pUI;
	m_Params.clickable = true;
	m_Active = false;
	m_BlinkTimer = 0.0f;
}


bool TextInput::VHandleClick()
{
	if (m_Active)
	{
		m_pUI->ClearKeyHook();
		m_Active = false;
		m_Text.m_Text = m_Txt;
	} else
	{
		m_pUI->SetKeyHook(this);
		m_Active = true;
		m_BlinkTimer = 0.0f;
	}

	return false;
}

bool TextInput::VOnKey(char key)
{
	if (m_Active)
	{
		switch (key)
		{
		case 27:
			m_Active = false;
			m_pUI->ClearKeyHook();
			m_Text.m_Text = m_Txt;
			break;
		case 8:
			if (m_Txt.length() >= 1)
			{
				m_Txt.resize(m_Txt.length() - 1);
			}
			break;
		default:
			m_Txt += key;
		}
		return false;
	}

	return true;
}

void TextInput::VUpdate(float delta)
{
	if (m_Active)
	{
		m_BlinkTimer += delta;

		if (m_BlinkTimer < 1.0f)
		{
			m_Text.m_Text = m_Txt + "|";
		} else if (m_BlinkTimer < 2.0f)
		{
			m_Text.m_Text = m_Txt;
		} else
		{
			m_BlinkTimer -= 2.0f;
			m_Text.m_Text = m_Txt + "|";
		}
	}
}