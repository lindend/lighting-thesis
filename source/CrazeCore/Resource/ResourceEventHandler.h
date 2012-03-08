#pragma once

#include "Resource.h"

namespace Craze
{
	class Resource;
	struct ResourceLoadData;

	class ResourceEventHandler
	{
	public:
        ResourceEventHandler() : m_supportsReload(true) {}
		virtual ~ResourceEventHandler() {}

		//Returning false from any of the handlers below will cancel the file loading and flag the resource with an error

		//Called before the resource starts to load, from the thread that started the loading of the resource
		virtual bool preRead(Resource* res) = 0;
		//Called once the resource has been fully loaded, this is when you want to start loading subresources
		virtual bool readComplete(ResourceLoadData* resData) = 0;
		//Called when all of the subresources has been fully loaded
		virtual bool allComplete(ResourceLoadData* resData) = 0;

		//If there are errors reading the file, this function will be called. Return true from this to continue processing anyway
		virtual bool fileReadError(ResourceLoadData* resData) = 0;

		virtual Resource* createResource(u32 typeId, u64 resId) = 0;

		bool readCompleteMT() const { return m_readCompleteMT; }
		bool allCompleteMT() const { return m_allCompleteMT; }
		bool supportsReload() const { return m_supportsReload; }
	protected:
		bool m_readCompleteMT;
		bool m_allCompleteMT;
		bool m_supportsReload;
	};

	class DefaultResHandler : public ResourceEventHandler
	{
	public:
		DefaultResHandler() { m_readCompleteMT = true; m_allCompleteMT = true;}

		virtual bool preRead(Resource* ) { return true; }
		virtual bool readComplete(ResourceLoadData* resData);
		virtual bool allComplete(ResourceLoadData* ) { return true; }

		virtual bool fileReadError(ResourceLoadData* ) { return true; }

		virtual Resource* createResource(u32, u64) { return CrNew DefaultResource(); }

	};
}
