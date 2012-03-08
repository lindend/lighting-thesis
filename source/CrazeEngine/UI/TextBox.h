#pragma once
#ifndef CRAZE__TEXT__BOX__H__
#define CRAZE__TEXT__BOX__H__

#include "IComponent.h"
#include "Graphics/Text/Text.h"

namespace Craze
{
	namespace Graphics
	{
		namespace UI
		{
			class TextBox : public IComponent
			{
			public:
				TextBox(IDevice* pDevice, std::string name = "");
				TextBox(IDevice* pDevice, float xpos, float ypos, std::string name = "");
				TextBox(IDevice* pDevice, float xpos, float ypos, float width, float height, std::string name = "");

				void SetText(const std::string& str);
				const std::string& GetText() const;

				const Fonts::FontProperties& GetFontProperties() const;
				void SetFontProperties(const Fonts::FontProperties& fp);

				virtual std::string VGetName();
				virtual ComponentParams* VGetParams();

				virtual bool VHandleClick();

			protected:
				virtual void VDraw(Renderer::Renderer* pRenderer);
				virtual void VUpdate(float delta);
				virtual bool VIsOver(float xpos, float ypos);

				ComponentParams m_Params;

				float m_Depth;

				std::string m_Name;

				Material m_Material;

				Fonts::Text m_Text;
				Fonts::FontProperties m_FontProps;
				
				IDevice* m_pDevice;
			};
		}
	}
}

#endif /*CRAZE__TEXT__BOX__H__*/