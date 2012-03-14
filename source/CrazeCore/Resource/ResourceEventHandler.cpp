#include "ResourceEventHandler.h"
#include "Resource.h"

using namespace Craze;

bool DefaultResHandler::readComplete(ResourceLoadData* resData)
{
	std::shared_ptr<DefaultResource> res = std::dynamic_pointer_cast<DefaultResource>(resData->res);
	if (!res)
	{
	    return false;
	}
	res->m_data = resData->data;
	res->m_size = resData->dataSize;
	resData->data = nullptr;
	return true;
}
