#pragma once
#include <string>

#include "Memory/MemoryManager.h"
#include "ResourceData.h"
#include "../Threading/ThreadUtil.hpp"

#include "../CrazeMath/MathTypes.h"

namespace Craze
{
	class ResourceManager;
	class ResourceEventHandler;
	class ResourceDataLoader;

	enum RES_LOAD_ERROR
	{
		RLE_SUCCESS,
		RLE_OUT_OF_MEMORY,
		RLE_OPEN_FILE,
		RLE_READING_DATA,
		RLE_LOADING_CHILD,

		RLE_HANDLER_READCOMPLETE,
		RLE_HANDLER_PREREAD,
		RLE_HANDLER_ALLCOMPLETE,
	};

	class Resource
	{
		//CRAZE_POOL_ALLOC(Resource);
		friend ResourceManager;
	public:
		enum RESOURCE_STATUS
		{
			READFILE,
			PROCESS,
			LOADSUB,
			FINISHED,
			LOADERROR,
			RELEASED,
			RELOAD,
		};

	public:
        void addRef() const { InterlockedIncrement((LONG*)&m_counter); }
		unsigned int release() const;

		u64 getFileId() const { return m_fileId; }

		const Resource** getDependencies() const { return m_dependencies; }
		unsigned int getNumDependencies() const { return m_numDependencies; }

		RES_LOAD_ERROR getError() const { return m_currentError; }
		RESOURCE_STATUS getStatus() const { return m_status; }

		const Event& getFinishedEvent() const { return m_finishedEvent; }

		const bool isLoaded() const { return getStatus() == FINISHED; }

		void setDependencies(const Resource** pDepenencies, unsigned int numDependencies);
		void setStatus(RESOURCE_STATUS status);
		void setError(RES_LOAD_ERROR error);
		void destroy();

		virtual ~Resource() {}

		std::string name;

    protected:
        virtual void onDestroy() = 0;

        Resource();
	private:
		u64 m_fileId;

		mutable volatile int m_counter;

		ResourceManager* m_resourceManager;

		const Resource** m_dependencies;
		unsigned int m_numDependencies;

		Event m_finishedEvent;

		RES_LOAD_ERROR m_currentError;
		RESOURCE_STATUS m_status;

		Resource(const Resource& o);
		Resource& operator=(const Resource& o);
	};

	class DefaultResource : public Resource
	{
		friend class DefaultResHandler;
	    CRAZE_POOL_ALLOC(DefaultResource);
    public:
        const char* getData() const { return m_data; }
        unsigned int getSize() const { return m_size; }
    protected:
        DefaultResource() : m_data(0), m_size(0) {}

        virtual void onDestroy()
        {
            delete [] m_data;
            m_data = 0;
        }
    private:
        const char* m_data;
        unsigned int m_size;
	};

	struct ResourceLoadData
	{
		CRAZE_POOL_ALLOC(ResourceLoadData);

	public:
		ResourceLoadData() : res(nullptr), data(nullptr), dataSize(0), evtHandler(nullptr), dataLoader(nullptr) {}

		Resource* res;

		char* data;
		int dataSize;

		ResourceEventHandler* evtHandler;
		ResourceDataLoader* dataLoader;

	private:
		ResourceLoadData(const ResourceLoadData& o);
		ResourceLoadData& operator=(const ResourceLoadData& o);
	};
}
