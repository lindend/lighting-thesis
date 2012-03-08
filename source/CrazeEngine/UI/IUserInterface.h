#pragma once
#ifndef CRAZE__USER__INTERFACE__INTERFACE__H__
#define CRAZE__USER__INTERFACE__INTERFACE__H__

#include "Graphics/Renderer/Renderer.h"
#include "IComponent.h"
#include "Button.h"
#include <set>

namespace Craze
{

	namespace Graphics
	{
		namespace UI
		{
			/**
			The base user interface used to render screen space objects.
			*/
			class IUserInterface
			{
			public:
				/**
				Renders all the visible screen components.
				*/
				virtual void VRender(Renderer::Renderer* pRenderer) = 0;

				/**
				Updates the interface.
				@param delta The time in milliseconds since the last update.
				*/
				virtual void VUpdate(float delta) = 0;

				/**
				Creates a button for the user interface. 
				@param xpos The x coordinate of the position. Specified in units, 1 unit = the whole screen.
				@param ypos The y coordinate of the position. Specified in units, 1 unit = the whole screen.
				@param width The width of the button. Specified in units, 1 unit = the whole screen.
				@param height The height of the button. Specified in units, 1 unit = the whole screen.
				@param pParent The parent of the object. The parent will affect an objects position and visibility.
				@param name The name that will be shown if this objects listens to mouse input and is clicked.
				@return The created button, or NULL if the operation was unsuccessful.
				*/
				virtual Button* VCreateButton(float xpos, float ypos, float width, float height, IComponent* pParent = NULL, std::string name = "") = 0;

			protected:
			};
		}
	}
}


#endif /*CRAZE__USER__INTERFACE__INTERFACE__H__*/