#pragma once
#ifndef CRAZE__EVENT__LISTENER__INTERFACE__H__
#define CRAZE__EVENT__LISTENER__INTERFACE__H__

#include "IEvent.h"

namespace Craze
{
	namespace Event
	{
		/**
		IEventListener is an interface used for classes that wish to receive notifications on
		events.
		*/
		class IEventListener
		{
		public:
			/**
			Called when an event that the listener listens to is sent.
			@param pEvent the event that is sent.
			*/
			virtual void VOnEvent(IEvent* pEvent) = 0;

			virtual ~IEventListener() {}
		protected:
		};

	}
}


#endif /*CRAZE__EVENT__LISTENER__INTERFACE__H__*/