#ifndef __LOCKABLE__HPP_INCLUDED__
#define __LOCKABLE__HPP_INCLUDED__

/*
Lockable
Simple interface to inherit from if multithreaded support is wanted.
Uses the win32

License:
MIT
*/

#ifdef _WIN32
#include <windows.h>
#endif //_WIN32


class Lockable
{
public:
	Lockable()
	{
		InitializeCriticalSection(&m_CritSect);
	}
	~Lockable()
	{
		DeleteCriticalSection(&m_CritSect);
	}

	inline void Lock()
	{
		EnterCriticalSection(&m_CritSect);
	}
	inline void Unlock()
	{
		LeaveCriticalSection(&m_CritSect);
	}
	
private:
	CRITICAL_SECTION m_CritSect;
};

#endif /*__LOCKABLE__HPP_INCLUDED__*/