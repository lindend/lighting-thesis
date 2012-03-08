#pragma once
#ifndef CRAZE__TEXT__INPUT__H__
#define CRAZE__TEXT__INPUT__H__

#include "TextBox.h"

namespace Craze
{
	namespace Graphics
	{
		namespace UI
		{
			class TextInput : public TextBox
			{
			public:
				TextInput(IDevice* pDevice, class UserInterface* pUI, std::string name = "");
				TextInput(IDevice* pDevice, UserInterface* pUI, float xpos, float ypos, std::string name = "");
				TextInput(IDevice* pDevice, UserInterface* pUI, float xpos, float ypos, float width, float height, std::string name = "");

				virtual bool VOnKey(char key);

				virtual bool VHandleClick();

			protected:
				virtual void VUpdate(float delta);

				float m_BlinkTimer;

				bool m_Active;

				std::string m_Txt;

				UserInterface* m_pUI;
			};
		}
	}
}


#endif /*CRAZE__TEXT__INPUT__H__*/