#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "Win32KeyMapper.h"
#include "Event/EventManager.h"

namespace Craze
{
	enum KEYCODE
	{
		KC_UNKNOWN = 0x00,
		KC_A = 'a',
		KC_B = 'b',
		KC_C = 'c',
		KC_D = 'd',
		KC_E = 'e',
		KC_F = 'f',
		KC_G = 'g',
		KC_H = 'h',
		KC_I = 'i',
		KC_J = 'j',
		KC_K = 'k',
		KC_L = 'l',
		KC_M = 'm',
		KC_N = 'n',
		KC_O = 'o',
		KC_P = 'p',
		KC_Q = 'q',
		KC_R = 'r',
		KC_S = 's',
		KC_T = 't',
		KC_U = 'u',
		KC_V = 'v',
		KC_W = 'w',
		KC_X = 'x',
		KC_Y = 'y',
		KC_Z = 'z',
		KC_ESCAPE = 0x01,
		KC_UPARROW = 0x10,
		KC_DOWNARROW = 0x11,
		KC_LEFTARROW = 0x12,
		KC_RIGHTARROW = 0x13,
		KC_F1,
		KC_F2,
		KC_F3,
		KC_F4,
		KC_F5,
		KC_F6,
		KC_F7,
		KC_F8,
		KC_F9,
		KC_F10,
		KC_F11,
		KC_F12,
	};

	enum MOUSEBUTTON
	{
		BTN_LEFT,
		BTN_RIGHT,
		BTN_MID,
		BTN_BUTTON4,
		BTN_BUTTON5,
	};

	enum KEYSTATE
	{
		KS_PRESS,
		KS_RELEASE,
		KS_TYPE,
	};

	bool InitInput(HWND hWnd);

	class CRAZEENGINE_EXP InputManager
	{
		friend bool InitInput(HWND hWnd);
	public:
		InputManager() 
		{ 
			memset(m_Keys, 0, sizeof(bool) * 255);
		}

		bool Initialize(unsigned int windowId);
		void Update();

		void SetMousePosition(const Vector2& v);
		Vector2 GetMousePosition();

		bool IsKeyDown(KEYCODE key) { return m_Keys[(char)key]; }

		void ParseRawInputKeyboard(RAWINPUT* pRI);
		void ParseRawInputMouse(RAWINPUT* pRI);

		void ParseInput(MSG msg);
		void ParseRawInput(HRAWINPUT hRI);

	private:
		Vector2 m_LastMouse;
		Vector2 m_PrevFrameMouse;
		Vector2 m_FrameMouseDelta;

		RAWINPUTDEVICE m_RiDevs[2];

		Win32KeyMapper m_KeyMapper;
		HWND m_hWnd;
		bool m_Keys[255];
	};

	extern CRAZEENGINE_EXP InputManager gInput;
}