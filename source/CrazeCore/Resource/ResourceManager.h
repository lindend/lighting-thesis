#pragma once

/*
This class is used to load resources from the hard drive. This is done by calling
the LoadResource method. The resources uses manual reference counting (i.e. you
have to call AddRef and Release manually). However, any resource dependencies are
automatically released when the main resource is released.

The resource manager uses a loader thread to load all the data. However, LoadResource
should only be called from one thread, it is not thread safe.

To customize loading, callbacks for different file types can be specified. These callbacks
will at the moment only be called by the loader thread. A to do is to enable these callbacks
to be able to be called from the main thread as well.

Ideas about features:
It would be nice to be able to receive a callback when a resource has been loaded

Known issues:
-Resource dependency graphs with cycles will probably work very bad, and some resources will never be
 considered loaded.

*/
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <functional>

#include "concurrent_queue.h"
#include "boost/pool/object_pool.hpp"
#include "windows.h"

#include "../Threading/ThreadUtil.hpp"
#include "ResourceEventHandler.h"
#include "Resource.h"
#include "../CrazeMath/MathTypes.h"

namespace Craze
{
	struct LoadEvent;
	class ThreadPool;

	class ResourceManager
	{
		friend Resource;
	public:
		ResourceManager();

		bool initialize(ThreadPool* tp);
		void shutdown();

		void update();

		void addDataLoader(ResourceDataLoader* dataLoader);

		//The file ending does include the dot, i.e. picture.png will give the ending .png
		void setResourceHandling(u32 fileType, ResourceEventHandler* handler);

		std::shared_ptr<const Resource> loadResource(u64 fileId, std::function<void(std::shared_ptr<const Resource>)> callback = std::function<void(std::shared_ptr<const Resource>)>(nullptr));
		std::shared_ptr<const Resource> loadResourceBlocking(u64 fileId);

		//This is a blocking function that just reads in the data of the requested file, the
		//data returned is owned by the caller so make sure to dealloc it with delete[].
		char* readFileData(u64 fileId, int& outSize);

		int getNumPending() const { return m_pendingResources; }

		void reloadResource(std::shared_ptr<Resource> res);

	private:
		void release(std::shared_ptr<Resource> resource);
		void process(ResourceLoadData* data);

		void readData(ResourceLoadData* data);
		bool dataReadError(ResourceLoadData* data);
		void handleProcess(ResourceLoadData* data, bool mainThread = false);
		bool handleLoadSubRes(ResourceLoadData* data, bool mainThread = false);

		void handleEvents();

		static void loadCallback(void* arg);

		ResourceEventHandler* findEventHandler(u32 resType);
		ResourceDataLoader* findDataLoader(u64 fileId);

		std::vector<ResourceDataLoader*> m_dataLoaders;
		std::map<u32, ResourceEventHandler*> m_resourceHandlers;
		std::map<u64, std::weak_ptr<Resource>> m_resources;
		std::map<void*, std::list<std::function<void(std::shared_ptr<const Resource>)>>> m_onLoadCallbacks;

		Concurrency::concurrent_queue<ResourceLoadData*> m_loadEvents;

		ThreadPool* m_threadPool;

		int m_pendingResources;
		Lock m_resourceLock;

		volatile bool m_running;

		DefaultResHandler m_defResHandler;

		ResourceManager(const ResourceManager&);
		ResourceManager& operator=(const ResourceManager&);
	};

	extern ResourceManager gResMgr;
};
