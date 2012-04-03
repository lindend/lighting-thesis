#include "CrazeEngine.h"
#include "Window.h"

#include "StrUtil.hpp"

using namespace Craze;

bool Window::initialize(int width, int height, bool fullscreen, const std::string& wndName, WNDPROC wndProc, int posX, int posY)
{
	m_width = width;
	m_height = height;
	m_fullscreen = fullscreen;
	
	HINSTANCE hinst = reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
	
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.lpszMenuName = 0;
	std::wstring wname = StrToW(wndName);
	wc.lpszClassName = wname.c_str();

	if (!RegisterClass(&wc))
	{
		LOGMSG("Could not register window class", LOGTYPE_CRITICAL);
		return false;
	}

	DWORD style, exstyle;

	if (!fullscreen)
	{
		style = WS_OVERLAPPEDWINDOW;
		exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	} else
	{
		style = WS_OVERLAPPEDWINDOW;
		exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	}

	RECT wRect;
	wRect.left = posX;
	wRect.top = posY;
	wRect.right = width + posX;
	wRect.bottom = height + posY;

	if (AdjustWindowRectEx(&wRect, style, false, exstyle) == 0)
	{
		char buf[1024];
		sprintf_s(buf, 1023, "%s", "Could not adjust window rect ");
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buf + strlen(buf), 900, 0); 
		LOG_ERROR(buf);
	}

	m_hwnd = CreateWindowExA(exstyle, wndName.c_str(), wndName.c_str(), style, posX, posY,
		wRect.right - wRect.left, wRect.bottom - wRect.top, NULL, NULL, hinst, NULL);

	SetWindowTextA(m_hwnd, wndName.c_str());

	ShowWindow(m_hwnd, SW_SHOW);
	UpdateWindow(m_hwnd);

	return true;
}