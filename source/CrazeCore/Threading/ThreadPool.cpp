#include "ThreadPool.h"
#include "../SystemInfo/SystemInfo.h"

using namespace Craze;

bool ThreadPool::Initialize()
{
	return Initialize(SystemInfo::GetNumProcessors(), 15);
}

bool ThreadPool::Initialize(int numThreads, int numIOThreads)
{
	m_pThreads = new Thread[numThreads];
	if (!m_pThreads)
	{
		return false;
	}
	m_NumThreads = numThreads;

	m_pIOThreads = new Thread[numIOThreads];
	if (!m_pIOThreads)
	{
		return false;
	}
	m_NumIOThreads = numIOThreads;

	m_Running = true;

	for (int i = 0; i < m_NumThreads; ++i)
	{
		m_pThreads[i].Start(&ThreadPool::BeginThread, this);
	}
	for (int i = 0; i < m_NumIOThreads; ++i)
	{
		m_pIOThreads[i].Start(&ThreadPool::BeginIOThread, this);
	}

	return true;
}

void ThreadPool::Shutdown()
{
	if (m_Running)
	{
		m_Running = false;

		WorkItem dummy;
		while (m_WorkQueue.try_pop(dummy)) {}

		for (int i = 0; i < m_NumThreads; ++i)
		{
			m_pThreads[i].Await();
		}

		for (int i = 0; i < m_NumIOThreads; ++i)
		{
			m_pIOThreads[i].Await();
		}

		delete[] m_pThreads;
		delete[] m_pIOThreads;
	}
}

void ThreadPool::Schedule(WorkFunction pFunct, void* pArg)
{
	if (!m_Running)
	{
		return;
	}

	WorkItem itm;
	itm.pFunct = pFunct;
	itm.pArg = pArg;
	
	m_WorkQueue.push(itm);
	//Increase the semaphore counter to signal that another work item is available
	m_HasWork.Release();
}
void ThreadPool::ScheduleIO(WorkFunction pFunct, void* pArg)
{
	if (!m_Running)
	{
		return;
	}

	WorkItem itm;
	itm.pFunct = pFunct;
	itm.pArg = pArg;

	m_IOWorkQueue.push(itm);
	m_IOHasWork.Release();
}

unsigned int __stdcall ThreadPool::BeginThread(void* pArg)
{
	ThreadPool* pTP = (ThreadPool*)pArg;
	pTP->ThreadMain();
	return 0;
}

unsigned int __stdcall ThreadPool::BeginIOThread(void* pArg)
{
	ThreadPool* pTP = (ThreadPool*)pArg;
	pTP->IOThreadMain();
	return 0;
}

void ThreadPool::ThreadMain()
{
	while (m_Running)
	{
		if (m_HasWork.Acquire(300))
		{
			WorkItem itm;
			if (!m_WorkQueue.try_pop(itm))
			{
				continue;
			}

			itm.pFunct(itm.pArg);
		}
	}
}

void ThreadPool::IOThreadMain()
{
	while (m_Running)
	{
		if (m_IOHasWork.Acquire(500))
		{
			WorkItem itm;
			if (!m_IOWorkQueue.try_pop(itm))
			{
				continue;
			}

			itm.pFunct(itm.pArg);
		}
	}
}