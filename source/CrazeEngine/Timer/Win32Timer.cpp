#include "CrazeEngine.h"
#include "Win32Timer.h"

using namespace Craze;

void Win32Timer::Start()
{
	QueryPerformanceCounter(&m_Start);
	m_Snap = m_Start;
}

double Win32Timer::Snap()
{
	LARGE_INTEGER lastSnap = m_Snap;
	QueryPerformanceCounter(&m_Snap);

	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (double)(m_Snap.QuadPart - lastSnap.QuadPart) / (double)freq.QuadPart;
}

double Win32Timer::Stop()
{
	LARGE_INTEGER stopTime;
	QueryPerformanceCounter(&stopTime);

	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (double)(stopTime.QuadPart - m_Start.QuadPart) / (double)freq.QuadPart;
}