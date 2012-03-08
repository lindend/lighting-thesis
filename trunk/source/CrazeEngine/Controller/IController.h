#pragma once
#ifndef CRAZE__CONTROLLER__INTERFACE__H__
#define CRAZE__CONTROLLER__INTERFACE__H__

#include "Event/EventManager.h"

namespace Craze
{
	namespace Controller
	{
		/**
		The controller interface.
		Controllers are used as connectors between the event manager and big sub systems, like the
		graphics engine, the sound engine or the physics engine.
		*/
		class IController
		{
		public:
			/**
			Initializes the controller
			@param pEventManager the eventmanager which the controller should use
			@return indicates if the initialization was successfull
			*/
			virtual bool VInitialize(Event::EventManager* pEventManager) { m_pEventManager = pEventManager; return true; }
			/**
			Shuts the controller down most likely called when the application is about to close.
			*/
			virtual void VShutdown() = 0;

			/**
			Called each frame to update the controller, the controller should update it's subsystems.
			@param delta indicates the difference in time in seconds since the last frame
			@return indicates success of the update.
			*/
			virtual bool VUpdate(float delta) = 0;
		protected:
			Event::EventManager* m_pEventManager;
		};
	}
}

#endif /*CRAZE__CONTROLLER__INTERFACE__H__*/