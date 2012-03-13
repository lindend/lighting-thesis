#include "FileDataLoader.h"

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <process.h>
#include <Shlwapi.h>

#include "Resource.h"
#include "../SystemInfo/SystemInfo.h"
#include "../StrUtil.hpp"
#include "../Util/CrazeHash.h"
#include "ResourceManager.h"

int getFileSize(const std::string& fileName)
{
	struct __stat64 fileSize;
	if (_stat64(fileName.c_str(), &fileSize) == 0)
	{
		return (int)fileSize.st_size;
	}
	return 0;
}

using namespace Craze;

FileDataLoader Craze::gFileDataLoader;

FileDataLoader::FileDataLoader()
{

}

bool FileDataLoader::initialize()
{
    m_thread.Start(&threadStart, this);
    m_running = true;
    return true;
}

void FileDataLoader::shutdown()
{
	if (m_running)
	{
		m_running = false;
		m_thread.Await();
	}
}

unsigned int __stdcall FileDataLoader::threadStart(void* arg)
{
	FileDataLoader* fdl = (FileDataLoader*)arg;
    fdl->threadMain();
	return 0;
}

u64 FileDataLoader::addFile(const std::string& fileName)
{
	u64 hash = hash64(fileName.c_str());
	m_fileIds.insert(std::make_pair(hash, fileName));
	return hash;
}

bool FileDataLoader::has(u64 fileId)
{
    ScopedLock sl(m_miscLock);
    auto it = m_fileIds.find(fileId);
    if (it == m_fileIds.end())
    {
        return false;
    }

    std::string fileName = it->second;
    std::ifstream file;
    for (auto i = m_roots.begin(); i != m_roots.end(); ++i)
    {
        const std::string path = *i + fileName;
        file.open(path);
        if (file.is_open())
        {
            //Update our stored path to the file
            it->second = path;
            file.close();
            return true;
        }
    }
    return false;
}

u32 FileDataLoader::getType(u64 fileId)
{
    auto it = m_fileIds.find(fileId);
    assert(it != m_fileIds.end());
    std::string fileName = it->second;

	std::string ending = fileName.substr(fileName.find_last_of('.'), std::string::npos);
    return hash32(ending.c_str(), ending.length());
}

int FileDataLoader::getSize(u64 fileId)
{
    auto it = m_fileIds.find(fileId);
    assert(it != m_fileIds.end());
    return getFileSize(it->second);
}

bool FileDataLoader::read(u64 fileId, char* dest, int destSize)
{
    auto it = m_fileIds.find(fileId);
    assert(it != m_fileIds.end());

    std::ifstream file;
    file.open(it->second, std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    file.read(dest, destSize);

    file.close();

    return true;
}

__int64 getLastChanged(const std::string& path)
{
	struct _stat64 stat;
	if (_stat64(path.c_str(), &stat) == 0)
	{
		return stat.st_mtime;
	}
	return 0;
}

void FileDataLoader::add(Resource* res)
{
	ScopedLock lcLock(m_lastChangeLock);
    auto item = m_lastChanged.find(res);
    if (item == m_lastChanged.end())
    {
        auto it = m_fileIds.find(res->getFileId());
		res->name = it->second;
        assert(it != m_fileIds.end());
		m_lastChanged.insert(std::make_pair(res, getLastChanged(it->second)));
    }
}

void FileDataLoader::remove(Resource* res)
{
	ScopedLock lcLock(m_lastChangeLock);
	m_lastChanged.erase(res);
}
struct LastChangedItem
{
    Resource* res;
    std::string file;
    i64 changed;
};

void FileDataLoader::threadMain()
{
	const int CachedIterations = 10;

	std::vector<LastChangedItem> fileWatches;
    while (m_running)
    {
		fileWatches.clear();
		fileWatches.reserve(m_lastChanged.size());

		{
			ScopedLock lcLock(m_lastChangeLock);
			for (auto i = m_lastChanged.begin(); i != m_lastChanged.end(); ++i)
			{
			    LastChangedItem itm;
			    itm.res = i->first;
			    itm.changed = i->second;
			    itm.file = m_fileIds[i->first->getFileId()];
				fileWatches.push_back(itm);
			}
		}

		for (int i = 0; (i < CachedIterations) & m_running; ++i)
		{
			for (auto j = fileWatches.begin(); j != fileWatches.end(); ++j)
			{
				i64 changed = getLastChanged(j->file);
				if (changed != j->changed && changed != 0)
				{
					j->changed = changed;
					{
						ScopedLock lcLock(m_lastChangeLock);
						m_lastChanged[j->res] = changed;
					}

					gResMgr.reloadResource(j->res);
				}
			}

			Sleep(200);
		}
    }
}
