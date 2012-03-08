#pragma once
#include "windows.h"

namespace Craze
{
	class Win32Clock
	{
	public:
		Win32Clock()
		{
			LARGE_INTEGER freq;
			if (!QueryPerformanceFrequency(&freq))
			{
				m_Freq = 1.f;
			} else
			{
				m_Freq = 1.f / ((float)freq.QuadPart / 1000000.0f);
			}

		}

		unsigned long GetClock()
		{
			LARGE_INTEGER clock;
			QueryPerformanceCounter(&clock);

			return (unsigned long)(clock.QuadPart);// * m_Freq);
		}

		float m_Freq;
	};
}