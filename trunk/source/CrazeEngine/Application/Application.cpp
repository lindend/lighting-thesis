#include "CrazeEngine.h"
#include "Application.h"

#include "State/GameStateManager.h"
#include "Game/Game.h"
#include "Timer/ITimer.h"

#include "Input/InputManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"
#include "SystemInfo/SystemInfo.h"
#include "Threading/ThreadPool.h"

#include "UI/UISystem.h"

#include "Event/GameEvents.h"

#include "EventLogger.h"
#include <ctime>

#include "Graphics.h"

#include "Script/ScriptManager.h"

#include "Game/ScriptGame.h"

#include "Shlobj.h"
#include "Knownfolders.h"

#ifdef _DEBUG
#include <iostream>
#endif

using namespace Craze;

bool Application::m_Shutdown = false;

float Application::m_LastDelta = 0.0f;

namespace Craze
{
	Application* gpApplication = nullptr;
}


Application::Application()
{
	m_Shutdown = false;
	m_Initialized = false;
	m_pGame = nullptr;
	m_pThreadPool = nullptr;
}

Application::~Application()
{
}

bool useFullScreen = false;
bool realFullScreen = false;

bool Application::Initialize(int argc, const char *argv[])
{

	unsigned int resX = useFullScreen ? 1920 : 960;
	unsigned int resY = useFullScreen ? 1080 : 540;

	gpApplication = this;

	if (!InitMemory(30 * MEGABYTE, 16 * MEGABYTE, 30 * MEGABYTE))
	{
		return false;
	}

	if (!EventLogger::initialize(L"craze.log", Craze::LOGTYPE_INFORMATION))
	{
		return false;
	}

	char date[30];
	char time[30];
	char timeAndDate[255];
	_strdate_s(date, 30);
	_strtime_s(time, 30);

	sprintf_s(timeAndDate, 255,"                        %s %s", date, time);


	EventLogger::print("\n\n****************************************************************");
	EventLogger::print("                    Craze engine starting up");
	EventLogger::print(timeAndDate);
	EventLogger::print("****************************************************************\n");

	if (!SystemInfo::Initialize())
	{
		LOG_CRITICAL("Unable to initialize system info");
		return false;
	}

	m_pThreadPool = new ThreadPool();
	if (!m_pThreadPool || !m_pThreadPool->Initialize())
	{
		LOG_CRITICAL("Unable to initialize thread pool");
		return false;
	}

	if (!gResMgr.initialize(m_pThreadPool))
	{
		LOG_CRITICAL("Unable to load resource manager");
		return false;
	}

	if (!gFileDataLoader.initialize())
	{
		LOG_CRITICAL("Unable to initialize file data loader");
		return false;
	}
	gFileDataLoader.addLocation("");
	gFileDataLoader.addLocation(".");
	gResMgr.addDataLoader(&gFileDataLoader);
	
	if (!VCreateWindow(resX, resY, !realFullScreen))
	{
		LOGMSG("Error while creating window", LOGTYPE_CRITICAL);
		return false;
	}
	
	if (!InitInput(m_hWnd))
	{
		LOG_CRITICAL("Could not initialize the input manager.");
		return false;
	}
	if (!Graphics2::InitGraphics(m_hWnd, m_WindowWidth, m_WindowHeight))
	{
		LOG_CRITICAL("Unable to initialize graphics engine");
		return false;
	}

	if (!gScriptManager.Initialize())
	{
		LOG_CRITICAL("Unable to initialize scripting system");
		return false;
	}

	m_pGame = VCreateGame();
	if (!m_pGame || !m_pGame->VInitialize())
	{
		LOG_CRITICAL("Unable to create game");
		return false;
	}

	LOG_INFO("Craze engine initialized successfully");

	ShowCursor(false);

	m_Initialized = true;
	return true;
}

Game* Application::VCreateGame()
{
	return CrNew ScriptGame("game.lua", gScriptManager.GetL());
}

LRESULT Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		Shutdown();
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Application::VCreateWindow(int width, int height, bool windowed)
{
	m_WindowWidth = width;
	m_WindowHeight = height;
	m_Windowed = windowed;

	m_hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = Application::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"Craze engine application";

	if (!RegisterClass(&wc))
	{
		LOGMSG("Could not register window class", LOGTYPE_CRITICAL);
		return false;
	}

	DWORD style, exstyle;

	if (windowed)
	{
		style = WS_OVERLAPPEDWINDOW;
		exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	} else
	{
		style = WS_POPUP;
		exstyle = WS_EX_TOPMOST;
	}

	RECT wRect;
	wRect.left = 0;
	wRect.top = 0;
	wRect.right = width;
	wRect.bottom = height;

	if (AdjustWindowRectEx(&wRect, style, false, exstyle) == 0)
	{
		char buf[1024];
		sprintf_s(buf, 1023, "%s", "Could not adjust window rect ");
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buf + strlen(buf), 900, 0); 
		LOG_ERROR(buf);
	}

	m_hWnd = CreateWindowExA(exstyle, "Craze engine application", "Craze engine application", style, 0, 0,
		wRect.right - wRect.left, wRect.bottom - wRect.top, NULL, NULL, m_hInstance, NULL);

	SetWindowTextA(m_hWnd, VGetGameName().c_str());

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return true;
}

void Application::VMessagePump()
{
	MSG msg;

	while (PeekMessage(&msg, m_hWnd, NULL, NULL, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			Shutdown();
		} else if (msg.message == WM_INPUT)
		{

			gInput.ParseRawInput((HRAWINPUT)msg.lParam);

		} else if(msg.message == WM_KEYDOWN)
		{
			if (msg.wParam == VK_F11)
			{
				m_DoProfile = true;
			} else if (msg.wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
				Shutdown();
			}
		}

		if (!Graphics2::ui_handleInput(msg))
		{
			switch(msg.message)
			{
			//Add other messages that the input should be informed of here...
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_KEYDOWN:
			case WM_MOUSEWHEEL:
			case WM_KEYUP:
			case WM_CHAR:
				{

					gInput.ParseInput(msg);
					break;
				}

			}
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Application::VMainLoop()
{
	LOG_INFO("Beginning main loop");

	if (!m_Initialized)
	{
		LOG_CRITICAL("Craze engine not initialized");
		return;
	}

	m_DoProfile = false;
	m_Timer.Start();

	float frame = 0;

	while (!m_Shutdown)
	{
		gMemory.ToggleFrame();

		VMessagePump();

		if (m_DoProfile)
		{
			gProfiler.BeginFrame("General frame");

#ifdef _DEBUG
			std::cout << "Generating profile frame." << std::endl;
#endif

		}

		//m_pResourceLoader->Process(20 * MEGABYTE);
		gResMgr.update();
		gInput.Update();

		float delta = (float)m_Timer.Snap();

		m_LastDelta = delta;

		m_pGame->VUpdate(delta);

		gScriptManager.Update(delta);

		Graphics2::gpGraphics->Render();

		if (m_DoProfile)
		{
			gProfiler.StopFrame();
			m_DoProfile = false;
		}

		//Sleep(20);

	}

	gProfiler.PrintStatisticsToFile("profiler.html");

	double totalTime = m_Timer.Stop();

}

void Application::VDestroy()
{
	if (m_pGame)
	{
		m_pGame->VDestroy();

		delete m_pGame;
		m_pGame = 0;
	}

	gScriptManager.Shutdown();
	
	Craze::Graphics2::ShutdownGraphics();

	gResMgr.shutdown();
	gFileDataLoader.shutdown();

	if (m_pThreadPool)
	{
		m_pThreadPool->Shutdown();
		delete m_pThreadPool;
		m_pThreadPool = nullptr;
	}

	gMemory.Shutdown();

	EventLogger::shutdown();

	StringHashType::CleanUp();
}

void Application::DisplayCursor(bool show)
{
	ShowCursor(show);
}

std::string Application::GetDataDirPath() const
{
	wchar_t* pOut;
	SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, 0, &pOut);

	std::string res = WToStr(pOut);

	CoTaskMemFree(pOut);

	return res;
}