#pragma once
#include <unordered_map>

#include "boost/pool/object_pool.hpp"

#include "Threading/ThreadUtil.hpp"

#ifdef _DEBUG
#define CRAZE_BOOKMARK_MEMORY
#endif

struct Align
{
	Align(size_t _align) { align = _align; }
	size_t align;
};

struct FrameAlloc
{
	FrameAlloc() {}
};

class PersistentAlloc
{
public:
	PersistentAlloc() {}
};

struct StackAlloc
{
	StackAlloc() {}
};

enum MEMSIZES
{
	KILOBYTE = 1024,
	MEGABYTE = 1024 * KILOBYTE,
	GIGABYTE = 1024 * MEGABYTE,
};


#define MEM_AUTO_MARK_STACK Craze::AutoMarker craze__local__auto__marker(gMemory.StackMark())

#define CrNew new(__FILE__, __LINE__)

#define CRAZE_ALLOC() \
    public: \
        static void* operator new(size_t size, const char* pFile, int line) \
        { \
            return Craze::gMemory.Allocate(size, pFile, line); \
        } \
        static void operator delete(void* pMem) \
        { \
            Craze::gMemory.Free(pMem); \
        } \
		static void operator delete(void* pMem, const char*, int) \
		{\
			Craze::gMemory.Free(pMem); \
        } \
    private: \
        static void* operator new(size_t size);

#define CRAZE_ALLOC_ALIGN(align) \
    public: \
        static void* operator new(size_t size, const char* pFile, int line) \
        { \
            return Craze::gMemory.AllocateAligned(align, size, pFile, line); \
        } \
        static void operator delete(void* pMem) \
        { \
            Craze::gMemory.FreeAligned(pMem); \
        } \
		static void operator delete(void* pMem, const char*, int) \
		{\
			Craze::gMemory.FreeAligned(pMem); \
        } \
    private: \
        static void* operator new(size_t size);

//Pool allocators
#define CRAZE_POOL_ALLOC(ClassName) \
    public: \
        static void* operator new(size_t size, const char* pFile, int line) \
        { \
			void* pMem = s_MemPool.malloc(); \
            Craze::gMemory.RegisterAlloc(pMem, size, pFile, line); \
			return pMem; \
        } \
        static void operator delete(void* pMem) \
        { \
			Craze::gMemory.UnregAlloc(pMem); \
            s_MemPool.free((ClassName*)pMem); \
        } \
		static void operator delete(void* pMem, const char*, int) \
		{\
			Craze::gMemory.UnregAlloc(pMem); \
			s_MemPool.free((ClassName*)pMem); \
        } \
    private: \
        static boost::object_pool<ClassName> s_MemPool; \
        static void* operator new(size_t size);

#define CRAZE_POOL_ALLOC_ALIGN(ClassName, Align) \
    public: \
        static void* operator new(size_t size, const char* pFile, int line) \
        { \
            void* pMem = s_MemPool.malloc(); \
            Craze::gMemory.RegisterAlloc(pMem, size, pFile, line); \
			return pMem; \
        } \
        static void operator delete(void* pMem) \
        { \
			Craze::gMemory.UnregAlloc(pMem); \
            s_MemPool.free((ClassName*)pMem); \
        } \
		static void operator delete(void* pMem, const char*, int) \
		{\
			Craze::gMemory.UnregAlloc(pMem); \
			s_MemPool.free((ClassName*)pMem); \
        } \
    private: \
        static boost::object_pool<ClassName, Craze::PoolAllocAlign<Align>> s_MemPool; \
        static void* operator new(size_t size);

#define CRAZE_POOL_ALLOC_IMPL(ClassName) boost::object_pool<ClassName> ClassName::s_MemPool
#define CRAZE_POOL_ALLOC_ALIGN_IMPL(ClassName, Align) boost::object_pool<ClassName, Craze::PoolAllocAlign<Align>> ClassName::s_MemPool

//Note to self; read this: http://www.gamedev.net/topic/600891-memory-manager/page__pid__4804130
namespace Craze
{
	typedef unsigned long StackMarker;

	bool InitMemory(unsigned long persistentSize, unsigned long frameSize, unsigned long stackSize);

	inline void* Next16ByteAddr(void* pAddr) { return (void*)(((unsigned int)pAddr + 15) & (~0x0F)); }


	class MemoryManager
	{
		friend bool InitMemory(unsigned long persistentSize, unsigned long frameSize, unsigned long stackSize);
	public:
		void* Allocate(size_t size, const char* file, int line);
		void* AllocateAligned(int alignment, size_t size, const char* file, int line);
		void Free(void* pData);
		void FreeAligned(void* pData);

		void RegisterAlloc(void* pData, size_t size, const char* file, int line);
		void UnregAlloc(void* pData);

		StackMarker StackMark();
		void StackFree(StackMarker marker);
		void* StackAlloc(size_t size);
		void* StackAlloc(Align, size_t size);

		void* PersistentAlloc(size_t size);

		void* FrameAlloc(size_t size);

		void ToggleFrame();

		void Shutdown();

		MemoryManager();
		~MemoryManager() { Shutdown(); }

	private:
		
#ifdef CRAZE_BOOKMARK_MEMORY
		struct MemAllocInfo
		{
			const char* file;
			int line;
			int size;
		};
		std::unordered_map<void*, MemAllocInfo> m_Allocations;
		Lock m_AllocRegLock;
#endif
		char* m_pPersistent;
		unsigned long m_PersistentUsed;
		unsigned long m_PersistentSize;

		char* m_pFrameMem;
		unsigned long m_FrameUsed;
		unsigned long m_FrameSize;

		char* m_pStackMem;
		unsigned long m_StackUsed;
		unsigned long m_StackSize;

		bool m_IsInited;
	};

	extern MemoryManager gMemory;

    template <int Align> struct PoolAllocAlign
    {
        typedef size_t size_type;
        typedef size_t difference_type;

        static char* malloc(const size_t bytes)
        {
            return (char*)gMemory.AllocateAligned(Align, bytes, __FILE__, __LINE__);
        }
        static void free(char* const block)
        {
            return gMemory.FreeAligned(block);
        }
    };
    typedef PoolAllocAlign<16> PoolAllocAlign16;

	class AutoMarker
	{
	public:
		AutoMarker(StackMarker sm) : m_SM(sm) {}
		~AutoMarker() { gMemory.StackFree(m_SM); }
	private:
		StackMarker m_SM;
	};
}
