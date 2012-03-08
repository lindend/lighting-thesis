#pragma once

#include "sys/time.h"

namespace Craze
{
	
	class UnixClock
	{
	public:
		unsigned long GetClock()
		{
			timeval tme;
			gettimeofday(&tme, 0);
			return tme.tv_usec + tme.tv_sec * 1000000;
		}
	};
}
