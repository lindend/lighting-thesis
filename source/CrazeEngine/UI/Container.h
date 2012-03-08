#pragma once
#ifndef CRAZE__UI__CONTAINER__H__
#define CRAZE__UI__CONTAINER__H__

#include "IComponent.h"
#include "Math/Matrix4.h"

namespace Craze
{
	namespace Graphics
	{
		namespace UI
		{
			class Container : public IComponent
			{
			public:
				Container(std::string name);

				/**
				@return The name of the component.
				*/
				virtual std::string VGetName();
				/**
				@return The component parameters. The paramters should be editable.
				*/
				virtual ComponentParams* VGetParams();
				/**
				Renders the component
				@param pRenderer The renderer that should receive the input data.
				*/
				virtual void VDraw(Renderer::Renderer* pRenderer);
				/**
				Updates the component, called once each frame.
				@param delta The time in milliseconds since the last frame.
				*/
				virtual void VUpdate(float delta);

				virtual bool VHandleClick() { return false; }

			protected:

				/**
				Checks if the position requested is over the current component.
				@param xpos The x-position to check.
				@param ypos The y-position to check.
				@return True if the position is over the current component.
				*/
				virtual bool VIsOver(float xpos, float ypos) { return true; }

				
				std::string m_Name;
				ComponentParams m_Params;
			};
		}
	}
}

#endif /*CRAZE__UI__CONTAINER__H__*/