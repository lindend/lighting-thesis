#include "ResourceManager.h"
#include "EventLogger.h"
#include "StrUtil.hpp"
#include <fstream>

#include "ResourceDataLoader.h"
#include "../Threading/ThreadPool.h"

#include "assert.h"

using namespace Craze;

using namespace std;

namespace Craze
{
	ResourceManager gResMgr;
}

struct ResWorkItemArg
{
	CRAZE_POOL_ALLOC(ResWorkItemArg);
public:
	ResourceManager* resMgr;
	ResourceLoadData* loadData;
};
CRAZE_POOL_ALLOC_IMPL(ResWorkItemArg);

ResourceManager::ResourceManager() : m_running(false)
{
}

bool ResourceManager::initialize(ThreadPool* tp)
{
	m_threadPool = tp;
	m_pendingResources = 0;
	m_running = true;

	return true;
}

void ResourceManager::shutdown()
{
	m_running = false;
	ScopedLock sl(m_resourceLock);
	while (m_resources.size() > 0)
	{
		auto i = m_resources.begin();
		i->second->getFinishedEvent().Await();
		release(i->second);
	}
}

void ResourceManager::update()
{
	if (!m_running)
	{
		return;
	}
	handleEvents();
}

bool ResourceManager::dataReadError(ResourceLoadData* loadData)
{
	Resource* res = loadData->res;

	if (loadData->evtHandler->fileReadError(loadData))
	{
		return true;
	}

	res->setStatus(Resource::LOADERROR);

	return false;
}

void ResourceManager::readData(ResourceLoadData* loadData)
{
	Resource* res = loadData->res;

	assert(res != nullptr);
	assert(res->getError() == RLE_SUCCESS);

	int size = loadData->dataLoader->getSize(res->getFileId());
	loadData->dataSize = size;
	loadData->data = new char[size];

	if (!loadData->data)
	{
        res->m_currentError = RLE_OUT_OF_MEMORY;
        return;
	}

	if (!loadData->dataLoader->read(res->getFileId(), loadData->data, size))
	{
	    if (!dataReadError(loadData))
	    {
            res->m_currentError = RLE_READING_DATA;
            return;
	    }
	} else
	{
		res->setStatus(Resource::PROCESS);
	}
}

void ResourceManager::handleProcess(ResourceLoadData* loadData, bool mainThread)
{
	Resource* res = loadData->res;

	assert(res->getStatus() == Resource::PROCESS);

	if (loadData->evtHandler->readCompleteMT() || mainThread)
	{
		if (loadData->evtHandler->readComplete(loadData))
		{
			res->setStatus(Resource::LOADSUB);
		} else
		{
			res->setError(RLE_HANDLER_READCOMPLETE);
		}

		delete [] loadData->data;
		loadData->data = nullptr;
	}
}

bool ResourceManager::handleLoadSubRes(ResourceLoadData* loadData, bool mainThread)
{
	Resource* res = loadData->res;
	assert(res->getStatus() == Resource::LOADSUB);

	const Resource** dependencies = res->getDependencies();
	for (unsigned int i = 0; i < res->getNumDependencies(); ++i)
	{
		Resource::RESOURCE_STATUS status = dependencies[i]->getStatus();
		if (status == Resource::LOADERROR)
		{
			res->setError(RLE_LOADING_CHILD);
			return true;
		} else if(status != Resource::FINISHED)
		{
			ResWorkItemArg* arg = CrNew ResWorkItemArg;
			arg->resMgr = this;
			arg->loadData = loadData;
			m_threadPool->ScheduleIO(&ResourceManager::loadCallback, arg);
			return false;
		}
	}

	if (loadData->evtHandler->allCompleteMT() || mainThread)
	{
		if (!loadData->evtHandler->allComplete(loadData))
		{
			res->setError(RLE_HANDLER_ALLCOMPLETE);
		} else
		{
			res->setStatus(Resource::FINISHED);

		}
	}
	return true;
}

void ResourceManager::process(ResourceLoadData* loadData)
{
	Resource* res = loadData->res;

	if (res->getStatus() == Resource::RELOAD)
	{
        res->setStatus(Resource::READFILE);
	}

	if (res->getStatus() == Resource::READFILE)
	{
		readData(loadData);
	}

	if (res->getStatus() == Resource::PROCESS)
	{
		handleProcess(loadData);
	}

	if (res->getStatus() == Resource::LOADSUB)
	{
		if (!handleLoadSubRes(loadData))
		{
			return;
		}
	}

	m_loadEvents.push(loadData);
}

void ResourceManager::addDataLoader(ResourceDataLoader* dataLoader)
{
    ScopedLock sl(m_resourceLock);
    m_dataLoaders.push_back(dataLoader);
}

void ResourceManager::setResourceHandling(u32 fileType, ResourceEventHandler* handler)
{
	ScopedLock sl(m_resourceLock);

	auto itHandler = m_resourceHandlers.find(fileType);

	if (itHandler != m_resourceHandlers.end())
	{
		LOG_ERROR("The file type already has a resource handler set");
	} else
	{
		m_resourceHandlers.insert(std::make_pair(fileType, handler));
	}
}

ResourceDataLoader* ResourceManager::findDataLoader(u64 fileId)
{
	ResourceDataLoader* dataLoader = nullptr;
    for (auto i = m_dataLoaders.begin(); i != m_dataLoaders.end(); ++i)
    {
        if ((*i)->has(fileId))
        {
            dataLoader = *i;
            break;
        }
    }
	return dataLoader;
}

ResourceEventHandler* ResourceManager::findEventHandler(u32 fileType)
{
    auto handler = m_resourceHandlers.find(fileType);

	if (handler != m_resourceHandlers.end())
	{
		return handler->second;
	}
	return &m_defResHandler;
}

const Resource* ResourceManager::loadResource(u64 fileId, std::function<void(const Resource*)> callback)
{
	if (!m_running)
	{
		return nullptr;
	}

	ScopedLock resLock(m_resourceLock);

	auto itRes = m_resources.find(fileId);
	if (itRes != m_resources.end())
	{
		itRes->second->addRef();
		if (callback)
		{
            auto status = itRes->second->getStatus();
			if (status == Resource::FINISHED || status == Resource::LOADERROR)
			{
				callback(itRes->second);
			} else
			{
				m_onLoadCallbacks[itRes->second].push_back(callback);
			}
		}
		return itRes->second;
	}

	ResourceDataLoader* dataLoader = findDataLoader(fileId);

    if (!dataLoader)
    {
        char buf[255];
		sprintf_s(buf, "Unable to locate the resource: %u", fileId);
		//itoa(buf, fileId);
        LOG_ERROR(buf);
        return nullptr;
    }

    u32 type = dataLoader->getType(fileId);

	ResourceLoadData* loadData = CrNew ResourceLoadData();
	loadData->dataLoader = dataLoader;
	loadData->evtHandler = findEventHandler(dataLoader->getType(fileId));

	Resource* res = loadData->evtHandler->createResource(type, fileId);
	res->m_resourceManager = this;
	res->m_fileId = fileId;
	res->setStatus(Resource::READFILE);

	loadData->res = res;
	loadData->dataLoader->add(res);

	if (!loadData->evtHandler->preRead(res))
	{
		delete res;
		res = nullptr;
		delete loadData;
	} else
	{
		if (callback)
		{
			m_onLoadCallbacks[res].push_back(callback);
		}

		m_resources.insert(std::make_pair(fileId, res));

		ResWorkItemArg* arg = CrNew ResWorkItemArg;
		arg->resMgr = this;
		arg->loadData = loadData;
		m_threadPool->ScheduleIO(&ResourceManager::loadCallback, arg);
	}

	++m_pendingResources;

	return res;
}

void ResourceManager::reloadResource(Resource* res)
{
	if (!m_running)
	{
		return;
	}

	ScopedLock sl(m_resourceLock);

	ResourceDataLoader* dataLoader = findDataLoader(res->getFileId());
	ResourceEventHandler* evtHandler = findEventHandler(dataLoader->getType(res->getFileId()));
	if (evtHandler && !evtHandler->supportsReload())
    {
        return;
    }

    if (res->getStatus() != Resource::FINISHED && res->getStatus() != Resource::LOADERROR)
    {
        return;
    }

    res->setStatus(Resource::RELOAD);

	ResourceLoadData* loadData = CrNew ResourceLoadData();
	loadData->res = res;
	loadData->dataLoader = dataLoader;
	loadData->evtHandler = evtHandler;

	ResWorkItemArg* arg = CrNew ResWorkItemArg;
	arg->resMgr = this;
	arg->loadData = loadData;
	m_threadPool->ScheduleIO(&ResourceManager::loadCallback, arg);
}

char* ResourceManager::readFileData(u64 fileId, int& outSize)
{
	if (!m_running)
	{
		return nullptr;
	}

	ScopedLock sl(m_resourceLock);
	ResourceDataLoader* loader = findDataLoader(fileId);
	if (!loader)
	{
		return nullptr;
	}

	int size = loader->getSize(fileId);

	if (size == 0)
	{
		return nullptr;
	}

	char* pBuf = new char[size];
	if (loader->read(fileId, pBuf, size))
	{
		outSize = size;
		return pBuf;
	}

	delete [] pBuf;
	return nullptr;
}

const Resource* ResourceManager::loadResourceBlocking(u64 fileId)
{
	const Resource* res = loadResource(fileId);

	if (res)
	{
		while (!res->getFinishedEvent().Await(1))
		{
            update();
		}
	}

	return res;
}

void ResourceManager::release(Resource* res)
{
	ScopedLock resLock(m_resourceLock);

	m_resources.erase(res->getFileId());

	for (auto i = m_dataLoaders.begin(); i != m_dataLoaders.end(); ++i)
	{
		(*i)->remove(res);
	}

	if (res->getStatus() == Resource::LOADERROR || res->getStatus() == Resource::FINISHED)
	{
		res->destroy();
		delete res;
	} else
	{
		//Add it to a removal list and release when its status is LOADERROR or FINISHED
	}
}

void ResourceManager::handleEvents()
{
	ResourceLoadData* loadData = nullptr;
	while (m_loadEvents.try_pop(loadData))
	{
		Resource* res = loadData->res;

		if (res->getStatus() == Resource::PROCESS)
		{
			handleProcess(loadData, true);

			if (res->getStatus() == Resource::LOADSUB && loadData->evtHandler->allCompleteMT())
			{
				ResWorkItemArg* arg = CrNew ResWorkItemArg;
				arg->resMgr = this;
				arg->loadData = loadData;
				m_threadPool->ScheduleIO(&ResourceManager::loadCallback, arg);
				continue;
			}
		}

		if (res->getStatus() == Resource::LOADSUB)
		{
			if (!handleLoadSubRes(loadData, true))
			{
				continue;
			}
		}

		if (res->getStatus() == Resource::FINISHED || res->getStatus() == Resource::LOADERROR)
		{
			const auto it = m_onLoadCallbacks.find(res);
			if (it != m_onLoadCallbacks.end())
			{
				for (auto i = it->second.begin(); i != it->second.end(); ++i)
				{
					(*i)(loadData->res);
				}
				m_onLoadCallbacks.erase(it);
			}
			delete loadData;
			--m_pendingResources;
		}
	}
}

void ResourceManager::loadCallback(void* arg)
{
	ResWorkItemArg* resArg = (ResWorkItemArg*)arg;
	ResourceManager* resMgr = resArg->resMgr;
	ResourceLoadData* loadData = resArg->loadData;

	delete resArg;

	resMgr->process(loadData);
}
