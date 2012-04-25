#include "MemoryManager.h"
#include <sstream>

#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "nedmalloc.h"

#include "assert.h"
#include "EventLogger.h"
#include "../Threading/ThreadUtil.hpp"

using namespace Craze;

namespace Craze
{
	MemoryManager gMemory;
}
void* MemoryManager::Allocate(size_t size, const char* file, int line)
{
	void* pMem = malloc(size);
	RegisterAlloc(pMem, size, file, line);
	return pMem;
}

void* MemoryManager::AllocateAligned(int alignment, size_t size, const char* file, int line)
{
	void* pMem = _aligned_malloc(size, alignment);
	RegisterAlloc(pMem, size, file, line);
	return pMem;
}

void MemoryManager::Free(void* pData)
{
	UnregAlloc(pData);
	free(pData);
}

void MemoryManager::FreeAligned(void* pData)
{
	UnregAlloc(pData);
	_aligned_free(pData);
}

void MemoryManager::RegisterAlloc(void* pMem, size_t size, const char* file, int line)
{
#ifdef CRAZE_BOOKMARK_MEMORY
	ScopedLock sl(m_AllocRegLock);
	MemAllocInfo inf = {file, line, (int)size};
	m_Allocations.insert(std::make_pair(pMem, inf));
#endif
}
void MemoryManager::UnregAlloc(void* pMem)
{
#ifdef CRAZE_BOOKMARK_MEMORY
	if (m_IsInited)
	{
		ScopedLock sl(m_AllocRegLock);
		m_Allocations.erase(pMem);
	}
#endif
}

MemoryManager::MemoryManager()
{
	m_FrameUsed = 0;
	m_PersistentUsed = 0;
	m_StackUsed = 0;

	m_pFrameMem = 0;
	m_pStackMem = 0;
	m_pPersistent = 0;

	m_IsInited = false;
}

bool Craze::InitMemory(unsigned long persistentSize, unsigned long frameSize, unsigned long stackSize)
{
	if (gMemory.m_IsInited)
	{
		//Careful with this, the log system might not be initialized yet
		LOG_ERROR("The memory manager is already initialized!");

		return false;
	}

	gMemory.m_FrameSize = frameSize;
	gMemory.m_PersistentSize = persistentSize;
	gMemory.m_StackSize = stackSize;

	gMemory.m_pFrameMem = (char*)gMemory.Allocate(frameSize, __FILE__, __LINE__);
	gMemory.m_pStackMem = (char*)gMemory.Allocate(stackSize, __FILE__, __LINE__);
	gMemory.m_pPersistent = (char*)gMemory.Allocate(persistentSize, __FILE__, __LINE__);

	if (!(gMemory.m_pFrameMem && gMemory.m_pStackMem && gMemory.m_pPersistent))
	{
		gMemory.Shutdown();
		return false;
	}

	gMemory.m_IsInited = true;

	return true;
}

void* MemoryManager::FrameAlloc(size_t size)
{
	if (m_FrameUsed + size > m_FrameSize)
	{
		LOG_CRITICAL("Out of frame memory");
		return 0;
	}

	void* pMem = m_pFrameMem + m_FrameUsed;
	m_FrameUsed += size;

	return pMem;
}

void* MemoryManager::PersistentAlloc(size_t size)
{
	if (m_PersistentUsed + size > m_PersistentSize)
	{
		LOG_CRITICAL("Out of persistent memory");
		return 0;
	}

	void* pMem = m_pPersistent + m_PersistentUsed;
	m_PersistentUsed += size;

	return pMem;
}

void* MemoryManager::StackAlloc(size_t size)
{
	if (m_StackUsed + size > m_StackSize)
	{
		LOG_CRITICAL("Out of stack memory");
		return 0;
	}

	void* pMem = m_pStackMem + m_StackUsed;
	m_StackUsed += size;

	return pMem;
}

void* MemoryManager::StackAlloc(Align a, size_t size)
{
	void* pMem = StackAlloc(size + a.align - 1);
	size_t addr = (size_t)pMem;
	return (void*)((addr + a.align - 1) & ~a.align);
}

StackMarker MemoryManager::StackMark()
{
	return m_StackUsed;
}

void MemoryManager::StackFree(StackMarker m)
{
	if (m < m_StackUsed)
	{
		m_StackUsed = m;
	}
}

void MemoryManager::ToggleFrame()
{
	m_FrameUsed = 0;
}

void MemoryManager::Shutdown()
{

	Free(m_pStackMem);
	Free(m_pFrameMem);
	Free(m_pPersistent);

	m_pStackMem = 0;
	m_pFrameMem = 0;
	m_pPersistent = 0;

	m_IsInited = false;

#ifdef CRAZE_BOOKMARK_MEMORY
    return;
	for (auto i = m_Allocations.begin(); i != m_Allocations.end(); ++i)
	{
		std::stringstream sstream;
		sstream << "Unfreed memory. Size: " << i->second.size << ", File: " << i->second.file << ", Line: " << i->second.line;
		LOG_ERROR(sstream.str());
	}
#endif
}
