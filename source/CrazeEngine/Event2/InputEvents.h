#pragma once
#include "CrazeEngine.h"

#include "GlobalEvent.h"
#include "../Input/InputManager.h"

namespace Craze
{
	//Position
	typedef GlobalEvent1<const Vector2&, -1> MouseMove;

	//Position, button, pressed
	typedef GlobalEvent3<const Vector2&, MOUSEBUTTON, bool, -1> MouseButton;

	//Distance
	typedef GlobalEvent1<short, -1> MouseScroll;

	//Keycode, keystate
	typedef GlobalEvent2<KEYCODE, KEYSTATE, -1> Keyboard;
}