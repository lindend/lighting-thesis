#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

namespace Craze
{
	class IEvent;

	/**
	IEventHook is an interface for classes that wish to hook certain
	event types before they are sent out to the application.
	*/
	class CRAZEENGINE_EXP IEventHook
	{
	public:
		/**
		The callback for the hook, this will receive notifications about
		all events that the event hook is listening to.
		@param pEvent The event that was hooked.
		@return True if the event should be further processed by the game, false if
		the event should not be forwarded.
		*/
		virtual bool VOnEvent(IEvent* pEvent) = 0;

		virtual ~IEventHook() {}
	};

}


