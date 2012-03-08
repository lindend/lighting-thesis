#pragma once
#include "CrazeEngine.h"

namespace Craze
{

	class ITimer
	{
	protected:
	public:
		//Starts timing
		virtual void Start() = 0;
		//Returns the time in seconds between now
		//and the previous snap (or start if no
		//snap has been called).
		virtual double Snap() = 0;
		//Stops the timer and returns the time
		//since the timer was started in seconds.
		virtual double Stop() = 0;

		virtual ~ITimer(){ }
	};

}