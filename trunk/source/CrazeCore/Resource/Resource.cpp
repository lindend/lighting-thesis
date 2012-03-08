#include "Resource.h"
#include "ResourceManager.h"

#include "assert.h"

using namespace Craze;

//CRAZE_POOL_ALLOC_IMPL(Resource);
CRAZE_POOL_ALLOC_IMPL(DefaultResource);
CRAZE_POOL_ALLOC_IMPL(ResourceLoadData);

Resource::Resource() : m_counter(1), m_currentError(RLE_SUCCESS), m_status(READFILE), m_numDependencies(0), m_dependencies(nullptr),
	m_finishedEvent(false, true)
{
}

unsigned int Resource::release() const
{
	assert(m_counter > 0);

	if (InterlockedDecrement((LONG*)&m_counter) == 0)
	{
		m_resourceManager->release(const_cast<Resource*>(this));
	}

	return m_counter;
}

void Resource::destroy()
{
    onDestroy();
	for (unsigned int i = 0; i < m_numDependencies; ++i)
	{
		m_dependencies[i]->release();
	}
}

void Resource::setDependencies(const Resource** dependencies, unsigned int numDependencies)
{
	assert((numDependencies == 0 && dependencies == nullptr) || (numDependencies > 0 && dependencies != nullptr));

	if (m_dependencies)
	{
		for (unsigned int i = 0; i < m_numDependencies; ++i)
		{
			m_dependencies[i]->release();
		}

		delete m_dependencies;
		m_dependencies = 0;
	}

	m_dependencies = dependencies;
	m_numDependencies = numDependencies;
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
