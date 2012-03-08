#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "IPlayer.h"
#include "BaseManager/BaseManager.h"

namespace Craze
{
	typedef unsigned int PlayerId;

	/**
	PlayerManager
	A class to manage all the players, it automatically
	deals out unique ids for all players added.
	*/
	class CRAZEENGINE_EXP PlayerManager : public BaseManager<IPlayer>
	{
	public:
		/**
		Frees the memory of all objects and clears the object list.
		*/
		virtual void VClear()
		{
			PROFILE("PlayerManager::VClear");
			for (ObjectMap::iterator i = m_Objects.begin(); i != m_Objects.end(); ++i)
			{
				if (i->second)
				{
					((IPlayer*)i->second)->VDestroy();
					delete i->second;
				}
			}

			m_Objects.clear();

			
		}
	};
}