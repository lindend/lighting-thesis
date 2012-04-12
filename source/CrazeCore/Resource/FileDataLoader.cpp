#include "FileDataLoader.h"

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <process.h>
#include <Shlwapi.h>

#include "Resource.h"
#include "SystemInfo/SystemInfo.h"
#include "StrUtil.hpp"
#include "Util/CrazeHash.h"
#include "ResourceManager.h"
#include "EventLogger.h"

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
	LOG_ERROR("File data loader: Unable to open file " + fileName);
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

void FileDataLoader::add(std::shared_ptr<Resource> res)
{
	ScopedLock lcLock(m_lastChangeLock);
    auto item = m_lastChanged.find(res.get());
    if (item == m_lastChanged.end())
    {
        auto it = m_fileIds.find(res->getFileId());
		res->name = it->second;
        assert(it != m_fileIds.end());
		LastChangedItem lastChanged;
		lastChanged.changed = getLastChanged(it->second);
		lastChanged.res = res;
		lastChanged.file = it->second;
		m_lastChanged.insert(std::make_pair(res.get(), lastChanged));
    }
}

void FileDataLoader::remove(std::shared_ptr<Resource> res)
{
	ScopedLock lcLock(m_lastChangeLock);
	m_lastChanged.erase(res.get());
}
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
			    LastChangedItem itm = i->second;
				std::shared_ptr<Resource> res = itm.res.lock();
				if (res)
				{
					itm.file = m_fileIds[res->getFileId()];
					fileWatches.push_back(itm);
				}
			}
		}

		for (int i = 0; (i < CachedIterations) & m_running; ++i)
		{
			for (auto j = fileWatches.begin(); j != fileWatches.end(); ++j)
			{
				i64 changed = getLastChanged(j->file);
				if (changed != j->changed && changed != 0)
				{
					std::shared_ptr<Resource> res = j->res.lock();
					if (res)
					{
						j->changed = changed;
						{
							ScopedLock lcLock(m_lastChangeLock);
						
							m_lastChanged[res.get()].changed = changed;
						}

						gResMgr.reloadResource(res);
					}
				}
			}

			Sleep(200);
		}
    }
}
