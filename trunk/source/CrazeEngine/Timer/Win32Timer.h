#pragma once
#include "EngineExport.h"

#include "windows.h"

namespace Craze
{

	class CRAZEENGINE_EXP Win32Timer
	{
	protected:
		LARGE_INTEGER m_Start;
		LARGE_INTEGER m_Snap;
	public:
		//Starts timing
		void Start();
		//Returns the time in seconds between now
		//and the previous snap (or start if no
		//snap has been called).
		double Snap();
		//Stops the timer and returns the time
		//since the timer was started in seconds.
		double Stop();
	};

}