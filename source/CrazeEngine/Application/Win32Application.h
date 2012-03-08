#pragma once

#error "Win32Application is deprecated, do not include"

#include "IApplication.h"

namespace Craze
{
	class ITimer;
	class GameStateManager;

	/**
	Win32Application is the core class of the game implemented with win32 specific code. 
	It initializes the game and has control over the main loop.	Once the Application is 
	initialized a graphics system and a human view is created.
	*/
	class Win32Application
	{
	public:
		/**
		Win32Application constructor
		*/
		Win32Application() {}

		/**
		Handles the windows message pump.
		*/
		virtual void VMessagePump();

		/**
		The window procedure which handles the events we receive.
		@param hWnd the window handle
		@param msg the message type
		@wParam the wparam
		@lParam the lparam
		*/
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		/*
		Platform specific code
		*/

		/**
		Calls the sleep function of the operating system (Sleep on windows)
		@param milliseconds indicates the number of milliseconds to sleep
		*/
		virtual void VSleep(int milliseconds);

	protected:
		/**
		Creates the graphic engine object.
		@return a pointer to a Craze graphic engine.
		*/
		virtual void VCreateGraphicsEngine();

		/**
		Creates the timer object, should be overloaded by user.
		@return A timer interface pointer.
		*/
		virtual ITimer* VCreateTimer();

		/**
		Creates the application window.
		@param width the width of the window
		@param height the height of the window
		@param windowed if the application should use window mode
		@return indicates if the window was successfully created
		*/
		virtual bool VCreateWindow(int width, int height, bool windowed);

		/**
		Creates and initializes the applications input manager
		@return An input manager object
		*/
		virtual InputManager* VCreateInputManager();

		/**
		Creates and initializes the applications profiler with the platform specific timer
		@return A profiler object
		*/
		virtual Profiling::Profiler* VCreateProfiler();

		/**
		Shows or hides the cursor in the window.
		@param visible Indicates the visibility of the cursor.
		*/
		virtual void VShowCursor(bool visible);

		private:
			Win32Application(const Win32Application& app) { }
	};
}