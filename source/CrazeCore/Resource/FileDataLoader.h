#pragma once
#include <vector>
#include <map>
#include <set>

#include "ResourceDataLoader.h"
#include "../Threading/ThreadUtil.hpp"

namespace Craze
{
    class Resource;

	struct LastChangedItem
	{
		std::weak_ptr<Resource> res;
		std::string file;
		i64 changed;
	};


    class FileDataLoader : public ResourceDataLoader
    {
    public:
        FileDataLoader();
        bool initialize();
        void shutdown();

        virtual bool has(u64 resId);
        virtual u32 getType(u64 resId);
        virtual int getSize(u64 resId);
        virtual bool read(u64 resId, char* destBuf, int destSize);

        virtual void add(std::shared_ptr<Resource> res);
        virtual void remove(std::shared_ptr<Resource> res);

		void addLocation(const std::string& location)
		{
			ScopedLock sl(m_miscLock);
			m_roots.push_back(location);
		}

		u64 addFile(const std::string& fileName);

    private:
		static unsigned int __stdcall threadStart(void* arg);
        void threadMain();

        std::vector<std::string> m_roots;
        std::map<void*, LastChangedItem> m_lastChanged;
        std::map<u64, std::string> m_fileIds;

        Lock m_lastChangeLock;
		Lock m_miscLock;
		Thread m_thread;

        volatile bool m_running;

		FileDataLoader(const FileDataLoader&);
		FileDataLoader& operator=(const FileDataLoader&);
    };

	extern FileDataLoader gFileDataLoader;
}
