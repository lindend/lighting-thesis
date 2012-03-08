#pragma once
#include "concurrent_queue.h"

#include "ThreadUtil.hpp"

namespace Craze
{
	typedef void(*WorkFunction)(void*);

	struct WorkItem
	{
		WorkFunction pFunct;
		void* pArg;
	};

	class ThreadPool
	{
	public:
		ThreadPool() : m_Running(false), m_NumThreads(0), m_NumIOThreads(0) {}

		bool Initialize();
		bool Initialize(int numThreads, int numIOThreads);
		void Shutdown();

		void Schedule(WorkFunction pFunct, void* pArg);
		/*
		ScheduleIO is meant to be used for functions that blocks, waiting for IO. Currently it
		works exactly the same as a normal thread, there is just a lot more of the IO threads than
		the regular threads.
		*/
		void ScheduleIO(WorkFunction pFunct, void* pArg);

	private:
		static unsigned int __stdcall BeginThread(void* pArg);
		static unsigned int __stdcall BeginIOThread(void* pArg);
		void ThreadMain();
		void IOThreadMain();

		Concurrency::concurrent_queue<WorkItem> m_WorkQueue;
		Concurrency::concurrent_queue<WorkItem> m_IOWorkQueue;
		Semaphore m_HasWork;
		Semaphore m_IOHasWork;

		Thread* m_pThreads;
		int m_NumThreads;

		Thread* m_pIOThreads;
		int m_NumIOThreads;

		bool m_Running;


		ThreadPool(const ThreadPool&);
		ThreadPool& operator=(const ThreadPool&);
	};
}