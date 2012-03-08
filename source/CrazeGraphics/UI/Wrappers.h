#pragma once

#include "gwen/Controls/ImagePanel.h"

namespace Craze
{
	namespace Graphics2
	{
		class Image : public Gwen::Controls::ImagePanel
		{
		public:
			Image(Gwen::Controls::Base *pParent) : Gwen::Controls::ImagePanel(pParent) {}

			void SetColor(float r, float g, float b, float a) { SetDrawColor(Gwen::Color(r, g, b, a)); }
		};
	}
}