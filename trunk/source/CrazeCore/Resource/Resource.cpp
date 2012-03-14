#include "Resource.h"
#include "ResourceManager.h"

#include "assert.h"

using namespace Craze;

//CRAZE_POOL_ALLOC_IMPL(Resource);
CRAZE_POOL_ALLOC_IMPL(DefaultResource);
CRAZE_POOL_ALLOC_IMPL(ResourceLoadData);

Resource::Resource() : m_counter(1), m_currentError(RLE_SUCCESS), m_status(READFILE),
	m_finishedEvent(false, true)
{
}

void Resource::destroy()
{
    onDestroy();
}

void Resource::setDependencies(const std::vector<std::shared_ptr<const Resource>>& dependencies)
{
	m_dependencies = dependencies;
}


void Resource::setStatus(RESOURCE_STATUS status)
{
	m_status = status;

	if ((status == FINISHED) | (status == LOADERROR))
	{
		m_finishedEvent.Flag();
	}
}

void Resource::setError(RES_LOAD_ERROR error)
{
	m_currentError = error;
	setStatus(LOADERROR);
}
