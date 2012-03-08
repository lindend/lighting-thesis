#pragma once

#include "Memory/MemoryManager.h"

namespace Craze
{
    class ResourceData
	{
	public:
		virtual void Release() = 0;
		virtual ~ResourceData() {}

		virtual int VGetType() = 0;
	};

	class DefaultResData : public ResourceData
	{
		CRAZE_POOL_ALLOC(DefaultResData);
	public:
		DefaultResData() : m_pData(0) {}

		void Release()
		{
			delete[] m_pData;
			m_pData = 0;
		}

		~DefaultResData()
		{
			Release();
		}

		//This is basically begging for bugs ;]
		static int GetType()  { return 0x398A402B; }
		virtual int VGetType() { return GetType(); }

		const char* m_pData;
		int m_Size;

	private:
		DefaultResData(const DefaultResData&);
		DefaultResData& operator=(const DefaultResData&);
	};
}
