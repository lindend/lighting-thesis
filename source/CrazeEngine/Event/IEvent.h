#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "Stream/Stream.h"

#define DECL_CRAZE_EVENT public: virtual const EventType& VGetType() const { return m_EventTypeHash; } \
	private: static EventType m_EventTypeHash;

#define INIT_CRAZE_EVENT(eventName, className) EventType Craze::##className##::m_EventTypeHash( eventName );

namespace Craze
{

	typedef StringHashType EventType;

	
	/**
	IEvent is an interface for a event.
	*/
	class CRAZEENGINE_EXP IEvent
	{
	public:
		/**
		@return the type of the event. It's recommended that the event type is
		stored as a static member of the class so it doesn't have to be
		recalculated every time.
		*/
		virtual EventType VGetType() const = 0;

		//virtual IEvent* VClone() const = 0;

			
		/**
		Performs a dynamic cast to the specified type and returns a
		pointer.
		@return A pointer to this object of the new type.
		*/
		template <class T> T* Cast() { return dynamic_cast<T*>(this); }

		/*
		Destructor
		*/
		virtual ~IEvent() {}
	protected:

	};

}
