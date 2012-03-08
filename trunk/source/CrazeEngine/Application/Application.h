#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "../Timer/Win32Timer.h"

#include "Event/EventManager.h"

namespace Craze
{
	class GameStateManager;
	class Game;
	class ThreadPool;

	extern CRAZEENGINE_EXP class Application* gpApplication;

	/**
	The Application provides an interface for application classes. Application classes provide the very kernel of the
	game and is responsible for initializing most sub systems.
	*/
	class CRAZEENGINE_EXP Application
	{
	public:
		/**
		constructor
		*/
		Application();
		/**
		destructor
		*/
		virtual ~Application();

		/**
		Initializes most of the games subsystems
		@return Bool indicating success
		*/
		bool Initialize(int argc, const char* argv[]);

		/**
		Takes care of timing the frames as well as updating the subsystems
		*/
		virtual void VMainLoop();

		/**
		Closes the game after the current loop is finished
		*/
		static void Shutdown() { m_Shutdown = true; }

		/**
		Handles the shutdown event
		*/
		void EvtShutdown(EventPtr pEvt) { Shutdown(); }

		/**
		@return The delta in seconds since the last frame.
		*/
		float GetDelta();

		/**
		Way to find out the current game name. Should be overloaded by the game specific application class.
		@return A string with the game name
		*/
		virtual std::string VGetGameName() { return ""; }

		/**
		Frees all the objects used by the application.
		*/
		virtual void VDestroy();


		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		virtual std::string GetDataDirPath() const;


		/*********************
		Platform specific code
		**********************/

		/**
		Calls the sleep function of the operating system (Sleep on windows)
		@param milliseconds indicates the number of milliseconds to sleep.
		*/
		//void Sleep(int milliseconds);

	protected:
		/**
		Creates a window, should also fill in the applications window data
		@param width the width of the window
		@param height the height of the window
		@param windowed indicates if window mode should be used
		@return Shows if the window was successfully created.
		*/
		virtual bool VCreateWindow(int width, int height, bool windowed);

		/**
		Shows or hides the cursor in the window.
		@param visible Indicates the visibility of the cursor.
		*/
		void DisplayCursor(bool visible);

		/**
		Creates a game logic core
		@return The created game logic core
		*/
		virtual Game* VCreateGame();

		virtual void VMessagePump();

	protected:
		//A timer to check frame lengths.
		Win32Timer m_Timer;

		Game* m_pGame;

		ThreadPool* m_pThreadPool;

		//Window data
		unsigned int m_WindowWidth;
		unsigned int m_WindowHeight;
		HWND m_hWnd;

		//A module handle
		HINSTANCE m_hInstance;

		bool m_Initialized;
		bool m_DoProfile;
		bool m_Windowed;

	private:

		//Shutdown-flag
		static bool m_Shutdown;

		static float m_LastDelta;
	};


}