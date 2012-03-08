#include "CrazeEngine.h"
#include "EventManager.h"
#include "Profiling/Profiler.h"

using namespace Craze;

namespace Craze
{
	EventManager gEvtMgr;
}


Craze::EventManager::EventManager()
{
	m_CurrentQue = 0;

	m_CurrentId = 0;

	m_EventQues[0].clear();
	m_EventQues[1].clear();
}
ListenerId Craze::EventManager::AddListener(EventType type, EventCallback callback)
{
	PROFILE("EventManager::AddListener");

	EventListenerMap::iterator typeIt = m_Listeners.find(type);

	EventListenerList* pList;

	if (typeIt == m_Listeners.end())
	{
		pList = &(m_Listeners[type] = EventListenerList());
	} else
	{
		pList = &typeIt->second;
	}

	pList->insert(EventListenerListEntry(++m_CurrentId, callback));
	return m_CurrentId;

}

void Craze::EventManager::RemoveListener(Craze::EventType type, ListenerId id)
{
	PROFILE("EventManager::RemoveListener");
	EventListenerMap::iterator typeIt = m_Listeners.find(type);

	if (typeIt != m_Listeners.end())
	{
		typeIt->second.erase(id);
	}

}

ListenerId EventManager::AddHook(EventType type, HookCallback callback)
{
	PROFILE("EventManager::AddHook");

	EventHookMap::iterator typeIt = m_Hooks.find(type);

	EventHookList* pList;

	if (typeIt == m_Hooks.end())
	{
		pList = &(m_Hooks[type] = EventHookList());
	} else
	{
		pList = &typeIt->second;
	}

	pList->insert(EventHookListEntry(++m_CurrentId, callback));
	return m_CurrentId;
}

void EventManager::RemoveHook(EventType type, ListenerId id)
{
	PROFILE("EventManager::RemoveHook");
	EventHookMap::iterator typeIt = m_Hooks.find(type);

	if (typeIt != m_Hooks.end())
	{
		typeIt->second.erase(id);
	}

}

void Craze::EventManager::QueEvent(Craze::EventPtr pEvent, bool autoDelete)
{
	PROFILE("EventManager::QueEvent");

	if (!CheckHook(pEvent))
	{
		if (autoDelete)
		{
			delete pEvent;
			return;
		}
	}

	Craze::EventItem itm;
	itm.pEvent = pEvent;
	itm.autoDelete = autoDelete;

	GetCurrentQue()->push_back(itm);


}

void Craze::EventManager::ProcessEvent(Craze::EventPtr pEvent, bool autoDelete)
{
	PROFILE("EventManager::ProcessEvent");

	if (CheckHook(pEvent))
	{

		EventListenerMap::iterator typeIt = m_Listeners.find(pEvent->VGetType());

		if (typeIt != m_Listeners.end())
		{
			for (EventListenerList::iterator i = typeIt->second.begin(); i != typeIt->second.end(); ++i)
			{
				(*i).second(pEvent);

			}
		}
	}

	if (autoDelete)
	{
		delete pEvent;
	}


}

void Craze::EventManager::Update()
{
	PROFILE("EventManager::Update");

	m_CurrentQue++;

	if (m_CurrentQue == 2)
	{
		m_CurrentQue = 0;
	}

	EventList* pProcQue = GetLastQue();

	while(!pProcQue->empty())
	{
		EventItem itm = pProcQue->front();

		EventListenerMap::iterator typeIt = m_Listeners.find(itm.pEvent->VGetType());

		if (typeIt != m_Listeners.end())
		{
			for (EventListenerList::iterator i = typeIt->second.begin(); i != typeIt->second.end(); ++i)
			{
				(*i).second(itm.pEvent);

			}
		}
	

		if (itm.autoDelete)
		{
			delete itm.pEvent;
			itm.pEvent = 0;
		}

		pProcQue->pop_front();
	}
}



EventList* Craze::EventManager::GetCurrentQue()
{
	return &m_EventQues[m_CurrentQue];
}

EventList* Craze::EventManager::GetLastQue()
{
	if (m_CurrentQue == 0)
	{
		return &m_EventQues[1];
	}

	return &m_EventQues[0];
}

bool EventManager::AddTargetedListener(unsigned int receiverId, EventType type, EventCallback callback)
{
	PROFILE("EventManager::AddTargetedListener");
	TargetedEventListeners::iterator it = m_TargetedEventListeners.find(receiverId);

	if (it != m_TargetedEventListeners.end())
	{
		(*it).second.insert(EventCallbackListEntry(type, callback));
		return true;
	}

	(m_TargetedEventListeners[receiverId] = EventCallbackList()).insert(EventCallbackListEntry(type, callback));
	return true;

}


void EventManager::RemoveTargetedListener(unsigned int receiverId)
{
	PROFILE("EventManager::RemoveTargetedListener");
	m_TargetedEventListeners.erase(receiverId);
}


bool EventManager::SendTo(unsigned int receiverId, Craze::EventPtr pEvent, bool autoDelete)
{
	PROFILE("EventManager::SendTo");

	bool success = false;

	TargetedEventListeners::iterator it = m_TargetedEventListeners.find(receiverId);

	if (it != m_TargetedEventListeners.end())
	{
		
		EventCallbackList::iterator cback = it->second.find(pEvent->VGetType());

		if (cback == it->second.end())
		{
			cback = it->second.find(EVENTTYPE_ALL);

			if (cback != it->second.end())
			{
				cback->second(pEvent);
				success = true;
			}
		} else
		{
			cback->second(pEvent);
			success = true;
		}

	}

	if (autoDelete)
	{
		delete pEvent;
	}

	return success;
}

bool EventManager::CheckHook(Craze::EventPtr pEvent)
{
	PROFILE("EventManager::CheckHook");

	EventHookMap::iterator typeIt = m_Hooks.find(pEvent->VGetType());

	if (typeIt != m_Hooks.end())
	{
		for (EventHookList::iterator i = typeIt->second.begin(); i != typeIt->second.end(); ++i)
		{
			if (!(*i).second(pEvent))
			{
				return false;
			}

		}
	}
	
	return true;
}

void EventManager::Clear()
{
	PROFILE("EventManager::Clear");
	
	m_EventQues[0].clear();
	m_EventQues[1].clear();

}

EventManager::~EventManager()
{
	m_Hooks.clear();
	m_Listeners.clear();


}