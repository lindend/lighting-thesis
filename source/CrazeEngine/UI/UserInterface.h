#pragma once
#ifndef CRAZE__USER__INTERFACE__H__
#define CRAZE__USER__INTERFACE__H__

#include "IUserInterface.h"
#include "Graphics/Device/IDevice.h"
#include "Graphics/GraphicsResource/GraphicsResourceManager.h"
#include "Math/Matrix4.h"
#include "Event/EventManager.h"

namespace Craze
{
	namespace Graphics
	{
		namespace UI
		{
			class UserInterface
			{
			public:
				UserInterface(IDevice* pDevice, GraphicsResourceManager* pResMgr, Event::EventManager* pEventManager);

				~UserInterface();

				/**
				Renders all the visible screen components.
				*/
				void Render(Renderer::Renderer* pRenderer);

				//void SetCursor(std::string name);

				/**
				Updates the interface.
				@param delta The time in milliseconds since the last update.
				*/
				void Update(float delta);

				void ShowCursor(bool showCursor);

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
				Button* CreateButton(float xpos, float ypos, float width, float height, IComponent* pParent = NULL, std::string name = "");

				bool AddComponent(IComponent* pComponent, IComponent* pParent);
			
				bool OnClick(Event::EventPtr pEvent);
				bool OnMove(Event::EventPtr pEvent);
				bool OnKey(Event::EventPtr pEvent);

				void SetKeyHook(IComponent* pComponent);
				void ClearKeyHook();

			private:
				Math::Matrix4 m_ViewMatrix;
				IComponent* m_Root;
				IDevice* m_pDevice;

				IComponent* m_pKeyHook;

				Image* m_pCursor;

				GraphicsResourceManager* m_pResMgr;

				Event::EventManager* m_pEventManager;
				Event::ListenerId m_HookIdClick;
				Event::ListenerId m_HookIdMove;
				Event::ListenerId m_HookKey;

			};
		}
	}
}


#endif /*CRAZE__USER__INTERFACE__H__*/