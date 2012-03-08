#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "IEvent.h"
#include "Input/InputManager.h"

namespace Craze
{


	/**
	The event sent when the mouse moves. Built-in Craze engine event.
	*/
	class CRAZEENGINE_EXP MouseMoveEvent : public IEvent
	{
	public:
		/**
		Sets the current mouse position
		*/
		MouseMoveEvent(Craze::Vector2 pos) { m_Position = pos; }

		/**
		@return The event type stored in the static variable
		*/
		virtual EventType VGetType() const { return TYPE; }
		static const EventType TYPE;

		/**
		@return The current mouse position
		*/
		Vector2 GetPosition() const { return m_Position; }
	protected:

		Vector2 m_Position;
	};

	/**
	MouseDownEvent - Built-in Craze engine event.
	The event sent when a mouse button is pressed.
	*/
	class CRAZEENGINE_EXP MouseDownEvent : public IEvent
	{
	public:
		/**
		Sets the current mouse position and also what button that was pressed
		*/
		MouseDownEvent(Craze::Vector2 pos, MOUSEBUTTON btn) { m_Position = pos; m_Button = btn; }

		/**
		@return the event type.
		*/
		virtual EventType VGetType() const { return TYPE; }
		static const EventType TYPE;

		/**
		@return The current mouse position
		*/
		Vector2 GetPosition() const { return m_Position; }
		/**
		@return The button which was pressed
		*/
		MOUSEBUTTON GetButton() const { return m_Button; }
	protected:
		MOUSEBUTTON m_Button;
		Vector2 m_Position;
	};

	class CRAZEENGINE_EXP MouseUpEvent : public IEvent
	{
	public:
		/**
		Sets the current mouse position and also what button that was pressed
		*/
		MouseUpEvent(Craze::Vector2 pos, MOUSEBUTTON btn) { m_Position = pos; m_Button = btn; }

		/**
		@return the event type.
		*/
		virtual EventType VGetType() const { return TYPE; }
		static const EventType TYPE;

		/**
		@return The current mouse position
		*/
		Vector2 GetPosition() const { return m_Position; }
		/**
		@return The button which was pressed
		*/
		MOUSEBUTTON GetButton() const { return m_Button; }
	protected:
		MOUSEBUTTON m_Button;
		Vector2 m_Position;
	};

	class CRAZEENGINE_EXP MouseWheelEvent : public IEvent
	{
	public:
		MouseWheelEvent(short distance) : m_Distance(distance) {}

		/**
		@return the event type.
		*/
		virtual EventType VGetType() const { return TYPE; }
		static const EventType TYPE;

		short GetDistance() const { return m_Distance; }
	private:
		short m_Distance;
	};

	class CRAZEENGINE_EXP KeyEvent : public IEvent
	{
	public:
		KeyEvent(KEYCODE key, KEYSTATE state) { m_Key = key; m_State = state; }

		KEYCODE GetKey() const { return m_Key; }
		KEYSTATE GetState() const { return m_State; }

		static const EventType TYPE;
		virtual EventType VGetType() const { return TYPE; }

	private:
		KEYCODE m_Key;
		KEYSTATE m_State;
	};
}