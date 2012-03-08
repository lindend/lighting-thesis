#pragma once
#ifndef CRAZE__USER__INTERFACE__BUTTON__H__
#define CRAZE__USER__INTERFACE__BUTTON__H__

#include "Image.h"

#include <functional>

namespace Craze
{
	namespace Graphics
	{
		namespace UI
		{
			typedef std::tr1::function<void(class Button*)> ClickCallback;
			class Button : public Image
			{
			public:
				Button(IDevice* pDevice, std::string name = "");
				Button(IDevice* pDevice, float xpos, float ypos, std::string name = "");
				Button(IDevice* pDevice, float xpos, float ypos, float width, float height, std::string name = "");
				
				void SetClickCallback(ClickCallback callback);

				virtual bool VHandleClick();

			private:
				ClickCallback m_Callback;

			};
		}
	}
}


#endif /*CRAZE__USER__INTERFACE__BUTTON__H__*/