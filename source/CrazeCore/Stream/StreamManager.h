#pragma once
#include "CrazeEngine.h"

#include "BaseManager/BaseManager.h"
#include "Stream.h"

namespace Craze
{
	class StreamManager : public BaseManager<Stream>
	{
	public:
		virtual unsigned int VAddItem(Stream* pStream)
		{
			unsigned int id = BaseManager::VAddItem(pStream);
			pStream->SetId(id);
			return id;
		}
		virtual bool VUpdate(float delta)
		{
			PROFILE("StreamManager::Update");

			for (ObjectMap::iterator i = m_Objects.begin(); i != m_Objects.end(); ++i)
			{
				(*i).second->VUpdate();
			}

				

			return true;
		}
	};
}