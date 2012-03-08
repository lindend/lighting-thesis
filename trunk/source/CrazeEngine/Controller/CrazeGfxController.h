#pragma once
#ifndef CRAZE__CRAZE__GFX__CONTROLLER__H__
#define CRAZE__CRAZE__GFX__CONTROLLER__H__

#include "IController.h"
#include "Graphics/GraphicsEngine.h"

namespace Craze
{
	namespace Controller
	{
		/**
		Graphics controller using the craze graphics engine.
		*/
		class CrazeGfxController : public IController
		{
		public:
			CrazeGfxController(Graphics::GraphicsEngine* pGfxEngine);

			/**
			Sets the controllers event manager to pEventManager and sets up the events to listen for
			@return always true
			*/
			virtual bool VInitialize(Event::EventManager* pEventManager);
			/**
			Updates the graphics engine and renders the scene, also presents the back buffer
			@param the difference in seconds since last frame
			@return indicates success of the rendering
			*/
			virtual bool VUpdate(float delta);

			/**
			Closes the graphics engine and frees its resources
			*/
			virtual void VShutdown();

			/**
			@return The graphics engine object associated with the controller
			*/
			virtual Graphics::GraphicsEngine* VGetGraphicsEngine() { return m_pGraphicsEngine; }

			/**
			Handles the event sent by the event manager
			*/
			virtual void VOnEvent(Craze::Event::IEvent* pEvent);
		private:

			Graphics::GraphicsEngine* m_pGraphicsEngine;

		};
	}
}


#endif /*CRAZE__CRAZE__GFX__CONTROLLER__H__*/