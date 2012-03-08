#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

namespace Craze
{
	class IEvent;
	class EventType;

	class CRAZEENGINE_EXP EventTypes
	{
	private:
		EventTypes() {}
		~EventTypes() {}
		typedef std::map<EventType, IEvent*> EventTypeMap;

		EventTypeMap m_RegisteredEvents;
	public:
		static void RegisterEvent(EventType type, IEvent* pTemplate);
	};
}
