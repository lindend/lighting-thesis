#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "IEventHook.h"
#include "IEvent.h"

#define MEMFN(FunctionName) std::bind((&FunctionName), this, std::placeholders::_1)
#define MEMFN_PTR(FunctionName, pObj) std::bind(std::mem_fn(&( FunctionName )), pObj, std::placeholders::_1)


namespace Craze
{
	typedef IEvent* EventPtr;
		
	struct CRAZEENGINE_EXP EventItem
	{
		EventPtr pEvent;
		bool autoDelete;
	};

	const EventType EVENTTYPE_ALL = "ALL";

	typedef unsigned int ListenerId;

	typedef std::function<void(EventPtr)> EventCallback;
	typedef std::function<bool(EventPtr)> HookCallback;
	typedef std::list<EventItem> EventList;
	typedef std::list<ListenerId> ListenerIdList;


	/**
	EventManager is the messenger of the game engine. It provides a way to 
	send events to all systems that needs to know about certain events.
	In order to receive event notifications the class needs to inherit from
	the IEventListener and call AddListener for each of the event types that
	should be received.
	*/
	class CRAZEENGINE_EXP EventManager
	{
	public:
		EventManager();
		~EventManager();

		/**
		Adds a listener that listens to the specific event type.
		@param type the event type that the listener wants to receive
		@param pListener a pointer to a listener object
		@param ensureReceive if this is true the listener will get events even if an other
		listener has requested an exclusive state.
		@return The id assigned to the listener.
		*/
		ListenerId AddListener(EventType type, EventCallback callback);
		/**
		Removes a listener from the specified event type.
		@param type the type that the listener doesn't want to receive notifications from.
		@param id The id of the listener.
		*/
		void RemoveListener(EventType type, ListenerId id);

		/**
		Adds a listener for a certain receiver id.
		@param receiverId If an event targeted to this id the event will be sent to the corresponding listener.
		@param type The type of events that this callback should receive.
		@param callback The function to be called.
		*/
		bool AddTargetedListener(unsigned int receiverId, EventType type, EventCallback callback);

		/**
		Removes a listener from the list of targeted listeners.
		@param receiverId The receiver id to remove.
		*/
		void RemoveTargetedListener(unsigned int receiverId);

		/**
		Adds an event hook for the selected event type.
		@param type The type of the event to hook.
		@param pHookListener Pointer to the hook listener for the event type.
		@return The id of the hook.
		*/
		ListenerId AddHook(EventType type, HookCallback hook);

		/**
		Removes an event hook for the selected event type.
		@param type The type of event to unhook.
		@param id The id of the hook to remove.
		*/
		void RemoveHook(EventType type, ListenerId id);

		/**
		Ques an event to be processed at the next EventManager update.
		@param pEvent is the event that will be put in the que.
		@param autoDelete is a boolean that tells the system to
		automatically call delete on the event when the processing is done.
		*/
		void QueEvent(EventPtr pEvent, bool autoDelete = true);

		/**
		Processes and event immidiatly and does not wait for the EventManager update.
		@param pEvent is the event that will be processed.
		@param autoDelete is a boolean that tells the system to
		automatically call delete on the event when the processing is done.

		*/
		void ProcessEvent(EventPtr pEvent, bool autoDelete = true);

		/**
		Sends an event to a listener with a certain id.
		@param receiverId The id to send the event to.
		@param pEvent The event to send.
		*/
		bool SendTo(unsigned int receiverId, EventPtr pEvent, bool autoDelete = true);

		/**
		Processes all the events in the current que and sends them to their listeners.
		*/
		void Update();
	
		/**
		Clears all currently queued events.
		*/
		void Clear();

	protected:
		
		typedef std::map<ListenerId, EventCallback> EventListenerList;
		typedef std::pair<ListenerId, EventCallback> EventListenerListEntry;
		typedef std::map<EventType, EventListenerList> EventListenerMap;
		typedef std::pair<EventType, EventListenerList> EventListenerMapEntry;

		typedef std::map<ListenerId, HookCallback> EventHookList;
		typedef std::pair<ListenerId, HookCallback> EventHookListEntry;
		typedef std::map<EventType, EventHookList> EventHookMap;
		typedef std::pair<EventType, EventHookList> EventHookMapEntry;

		typedef std::map<EventType, EventCallback> EventCallbackList;
		typedef std::pair<EventType, EventCallback> EventCallbackListEntry;
		typedef std::map<unsigned int, EventCallbackList> TargetedEventListeners;

		/**
		Checks if there are any active hook on the event and polls the
		hooks if to continue sending the event.
		@param pEvent Pointer to the event to examine.
		@return True if the event should be sent through.
		*/
		bool CheckHook(EventPtr pEvent);

		/**
		@return the que used in the previous frame
		*/
		EventList* GetLastQue();
		/**
		@return the que used in the current frame
		*/
		EventList* GetCurrentQue();

		EventListenerMap m_Listeners;
		EventHookMap m_Hooks;
		TargetedEventListeners m_TargetedEventListeners;
		EventList m_EventQues[2];
		unsigned int m_CurrentQue;

		ListenerId m_CurrentId;
	};

	extern CRAZEENGINE_EXP EventManager gEvtMgr;
}
