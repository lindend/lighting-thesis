#pragma once

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include "process.h"

namespace Craze
{
	class Thread
	{
	public:
		Thread()
		{
		}

		void Start(unsigned int (__stdcall *pFunction)(void*), void* arg)
		{
			m_Thread = (HANDLE)_beginthreadex(NULL, 0, pFunction, arg, 0, nullptr);
		}

		bool Await(unsigned int timeOut = INFINITE) const
		{
			return WaitForSingleObject(m_Thread, timeOut) == WAIT_OBJECT_0;
		}

	private:
		HANDLE m_Thread;
	};
	class Lock
	{
	public:
		Lock()
		{
			InitializeCriticalSection(&m_Lock);
		}
		~Lock()
		{
			DeleteCriticalSection(&m_Lock);
		}

		void Acquire()
		{
			EnterCriticalSection(&m_Lock);
		}
		void Release()
		{
			LeaveCriticalSection(&m_Lock);
		}
	private:
		CRITICAL_SECTION m_Lock;

		Lock(const Lock&);
		Lock& operator=(const Lock&);
	};

	class ScopedLock
	{
	public:
		ScopedLock(Lock& lock) : m_Lock(lock)
		{
			m_Lock.Acquire();
		}
		~ScopedLock()
		{
			m_Lock.Release();
		}
	private:
		Lock& m_Lock;

		ScopedLock(const ScopedLock&);
		ScopedLock& operator=(const ScopedLock&);
	};

	class Event
	{
	public:
		Event(bool initial = false, bool manualReset = false)
		{
			m_Event = CreateEventW(NULL, manualReset, initial, NULL);
		}

		~Event()
		{
			CloseHandle(m_Event);
		}

		bool Await(unsigned int timeout = INFINITE) const
		{
			return WaitForSingleObject(m_Event, timeout) == WAIT_OBJECT_0;
		}

		void Flag()
		{
			SetEvent(m_Event);
		}

		void Reset()
		{
			ResetEvent(m_Event);
		}
	private:
		HANDLE m_Event;

		Event(const Event&);
		Event& operator=(const Event&);
	};

	class Semaphore
	{
	public:
		Semaphore(int maxCount = 100, int initialCount = 0)
		{
			m_Semaphore = CreateSemaphoreW(NULL, initialCount, maxCount, NULL);
		}
		~Semaphore()
		{
			CloseHandle(m_Semaphore);
		}
		bool Acquire(unsigned int timeout = INFINITE)
		{
			return WaitForSingleObject(m_Semaphore, timeout) == WAIT_OBJECT_0;
		}
		void Release()
		{
			ReleaseSemaphore(m_Semaphore, 1, NULL);
		}

	private:
		HANDLE m_Semaphore;

		Semaphore(const Semaphore&);
		Semaphore& operator=(const Semaphore&);
	};
}