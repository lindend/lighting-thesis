#pragma once
#include <string>

#include <Windows.h>

namespace Craze
{
	class Window
	{
	public:
		bool initialize(int width, int height, bool fullscreen, const std::string& wndName, WNDPROC wndProc, int posX = 0, int posY = 0);

		HWND getHwnd() { return m_hwnd; }
		unsigned int getWidth() { return m_width; }
		unsigned int getHeight() { return m_height; }

	private:
		unsigned int m_width;
		unsigned int m_height;
		HWND m_hwnd;
		bool m_fullscreen;
	};
}