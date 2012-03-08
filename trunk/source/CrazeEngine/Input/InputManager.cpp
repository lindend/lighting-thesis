#include "CrazeEngine.h"
#include "InputManager.h"
#include "Event/InputEvents.h"
#include "Event2/InputEvents.h"
#include "Win32KeyMapper.h"

using namespace Craze;

namespace Craze
{
	InputManager gInput;
}

bool Craze::InitInput(HWND hWnd)
{
	//Raw mouse
	gInput.m_RiDevs[0].usUsagePage = 0x01;
	gInput.m_RiDevs[0].usUsage = 0x02;
	gInput.m_RiDevs[0].dwFlags = 0;
	gInput.m_RiDevs[0].hwndTarget = hWnd;
	
	//Raw keyboard
	gInput.m_RiDevs[1].usUsagePage = 0x01;
	gInput.m_RiDevs[1].usUsage = 0x06;
	gInput.m_RiDevs[1].dwFlags = 0;
	gInput.m_RiDevs[1].hwndTarget = hWnd;

	if (RegisterRawInputDevices(gInput.m_RiDevs, 2, sizeof(gInput.m_RiDevs[0])) == false)
	{
		LOG_CRITICAL("Could not register raw input devices");
		return false;
	}

	gInput.m_LastMouse = gInput.GetMousePosition();
	gInput.m_hWnd = hWnd;

	return true;
}

void InputManager::Update()
{
	if (!(m_LastMouse == m_PrevFrameMouse))
	{
		//m_pEventManager->QueEvent(EventPtr(new MouseMoveEvent(m_PrevFrameMouse)));
		m_PrevFrameMouse = m_LastMouse;
	}

	m_FrameMouseDelta.x = m_FrameMouseDelta.y = 0;
}

void InputManager::ParseInput(MSG msg)
{
	if (msg.message == WM_MOUSEMOVE)
	{
		m_LastMouse.x = LOWORD(msg.lParam);
		m_LastMouse.y = HIWORD(msg.lParam);
		MouseMove::Notify(m_LastMouse);
		//m_pEventManager->ProcessEvent(new MouseMoveEvent(m_LastMouse));
	} else if(msg.message == WM_LBUTTONDOWN)
	{
		//m_pEventManager->QueEvent(new MouseDownEvent(m_LastMouse, BTN_LEFT));
		MouseButton::Notify(m_LastMouse, BTN_LEFT, true);

	} else if(msg.message == WM_RBUTTONDOWN)
	{
		//m_pEventManager->QueEvent(new MouseDownEvent(m_LastMouse, BTN_RIGHT));
		MouseButton::Notify(m_LastMouse, BTN_RIGHT, true);
	} 
	
	else if(msg.message == WM_LBUTTONUP)
	{
		//m_pEventManager->QueEvent(new MouseUpEvent(m_LastMouse, BTN_LEFT));
		MouseButton::Notify(m_LastMouse, BTN_LEFT, false);

	} else if(msg.message == WM_RBUTTONUP)
	{
		//m_pEventManager->QueEvent(new MouseUpEvent(m_LastMouse, BTN_RIGHT));
		MouseButton::Notify(m_LastMouse, BTN_RIGHT, false);
	}
	
	else if(msg.message == WM_MOUSEWHEEL)
	{
		//m_pEventManager->QueEvent(new MouseWheelEvent(GET_WHEEL_DELTA_WPARAM(msg.wParam)));
		MouseScroll::Notify(GET_WHEEL_DELTA_WPARAM(msg.wParam));
	}
	
	else if(msg.message == WM_CHAR)
	{
		//m_pEventManager->QueEvent(new KeyEvent(m_KeyMapper.ConvertKey((char)msg.wParam), KS_TYPE));
		Keyboard::Notify(m_KeyMapper.ConvertKey((char)msg.wParam), KS_TYPE);
	} 
	
	else if(msg.message == WM_KEYDOWN)
	{
		//m_pEventManager->QueEvent(new KeyEvent(m_KeyMapper.ConvertKey((char)msg.wParam), KS_PRESS));
		Keyboard::Notify(m_KeyMapper.ConvertKey((char)msg.wParam), KS_PRESS);

	} else if(msg.message == WM_KEYUP)
	{
		//m_pEventManager->QueEvent(new KeyEvent(m_KeyMapper.ConvertKey((char)msg.wParam), KS_RELEASE));
		Keyboard::Notify(m_KeyMapper.ConvertKey((char)msg.wParam), KS_RELEASE);
	} 
}

void InputManager::ParseRawInput(HRAWINPUT hRI)
{
	unsigned int itemSize;
	GetRawInputData(hRI, RID_INPUT, NULL, &itemSize, sizeof(RAWINPUTHEADER));

	if (itemSize == 0)
		return;

	char* buf = new char[itemSize];

	if(GetRawInputData(hRI, RID_INPUT, buf, &itemSize, sizeof(RAWINPUTHEADER)) != itemSize)
	{
		delete [] buf;
		return;
	}
	
	RAWINPUT* pRI = (RAWINPUT*)buf;

	if (pRI->header.dwType == RIM_TYPEMOUSE)
	{
		ParseRawInputMouse(pRI);

	} else if(pRI->header.dwType == RIM_TYPEKEYBOARD)
	{
		ParseRawInputKeyboard(pRI);
	}

	delete [] buf;

}

void InputManager::ParseRawInputKeyboard(RAWINPUT *pRI)
{
	if (pRI->data.keyboard.Message == WM_KEYDOWN)
	{
		m_Keys[m_KeyMapper.ConvertKey(pRI->data.keyboard.VKey)] = true;


	} else if(pRI->data.keyboard.Message == WM_KEYUP)
	{
		m_Keys[m_KeyMapper.ConvertKey(pRI->data.keyboard.VKey)] = false;
	}
}

void InputManager::ParseRawInputMouse(RAWINPUT *pRI)
{

	const RAWMOUSE* pMouse = &pRI->data.mouse;


	if(pMouse->usFlags == MOUSE_MOVE_RELATIVE)
	{
		m_FrameMouseDelta.x += (float)pMouse->lLastX;
		m_FrameMouseDelta.y += (float)pMouse->lLastY;
	}

	if (pMouse->usButtonFlags & RI_MOUSE_BUTTON_1_DOWN)
	{
		//m_pEventManager->VQueEvent(new MouseDownEvent(m_PrevFrameMouse, BTN_LEFT));

	} else if(pMouse->usButtonFlags & RI_MOUSE_BUTTON_1_UP)
	{
		
	}

	if (pMouse->usButtonFlags & RI_MOUSE_BUTTON_2_DOWN)
	{

	} else if(pMouse->usButtonFlags & RI_MOUSE_BUTTON_2_UP)
	{
	}

	if (pMouse->usButtonFlags & RI_MOUSE_BUTTON_3_DOWN)
	{

	} else if(pMouse->usButtonFlags & RI_MOUSE_BUTTON_3_UP)
	{
	}

	if (pMouse->usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
	{

	} else if(pMouse->usButtonFlags & RI_MOUSE_BUTTON_4_UP)
	{
	}

	if (pMouse->usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
	{

	} else if(pMouse->usButtonFlags & RI_MOUSE_BUTTON_5_UP)
	{
	}

	if (pMouse->usButtonFlags & RI_MOUSE_WHEEL)
	{

	}

}

void InputManager::SetMousePosition(const Vector2 &v)
{
	POINT p;
	p.x = (int)v.x;
	p.y = (int)v.y;
	ClientToScreen(m_hWnd, &p);
	SetCursorPos(p.x, p.y);
}

Vector2 InputManager::GetMousePosition()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(m_hWnd, &p);

	return Vector2((float)p.x, (float)p.y);
}