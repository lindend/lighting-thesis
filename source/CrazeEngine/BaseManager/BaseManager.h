#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

namespace Craze
{
	/**
	BaseManager is the base manager which can be extended into
	a container for other types of items.
	*/
	template <typename T> 
	class CRAZEENGINE_EXP BaseManager
	{
		typedef std::map<unsigned int, T*> ObjectMap;
		typedef std::set<unsigned int> UsedIdSet;
	public:

		BaseManager::BaseManager()
		{
			m_ItemCounter = 0;
		}
		/**
		Adds an item to the collection of items, Memory from added items
		will be freed when the list is cleared of the item is removed.
		@param item A pointer to the item to be added
		@return The assigned item Id needed to perform operations on the item.
		*/
		virtual unsigned int VAddItem(T* item)
		{
			PROFILE("BaseManager::VAddItem");
			if (!item)
			{
				
				return 0;
			}		

			while (m_UsedIds.find(m_ItemCounter + 1) != m_UsedIds.end())
			{
				++m_ItemCounter;
			}

			m_Objects[++m_ItemCounter] = item;

			
			return m_ItemCounter;
		}

		/**
		Tries to add an item in the specified index position. Memory from added items
		will be freed when the list is cleared of the item is removed.
		@param item the item to add
		@param index the desired item index
		@return indicates if the operation was successfull
		*/
		virtual bool VAddItem(T* item, unsigned int index)
		{
			PROFILE("BaseManager::VAddItem (set index)");
			if (!item)
			{
				
				return false;
			}
			
			if (m_UsedIds.find(index) != m_UsedIds.end())
			{
				
				return false;
			}

			m_UsedIds.insert(index);
			m_Objects[index] = item;
			
			
			return true;
		}

		/**
		Removes the item at the specified index.
		@param itemIndex the item index to remove an item at
		@return True if the operation succeeded, otherwise false.
		*/
		virtual bool VRemoveItem(unsigned int itemIndex)
		{
			PROFILE("BaseManager::VRemoveItem");
			std::map<unsigned int, T*>::iterator item = m_Objects.find(itemIndex);

			if (item != m_Objects.end())
			{
				m_Objects.erase(item);
				
				return true;
			}
			
			return false;
		}

		/**
		Returns an item at the specified item index.
		@return A pointer to the item at the specified location or null of the item could not be found.
		*/
		virtual T* VGetItem(unsigned int itemIndex)
		{
			PROFILE("BaseManager::VGetItem");

			std::map<unsigned int, T*>::iterator item = m_Objects.find(itemIndex);

			if (item == m_Objects.end())
			{
				
				return NULL;
		
			}
			
			return item->second;
		}

		/**
		Virtual function to update all managed objects each turn.
		@delta time in seconds since the last frame
		*/
		virtual bool VUpdate(float delta)
		{
			return false;
		}

		/**
		Frees the memory of all objects and clears the object list.
		*/
		virtual void VClear()
		{
			PROFILE("BaseManager::VClear");
			for (ObjectMap::iterator i = m_Objects.begin(); i != m_Objects.end(); ++i)
			{
				if (i->second)
				{
					delete i->second;
				}
			}

			m_Objects.clear();

			
		}

		virtual ~BaseManager()
		{
			VClear();
		}

	protected:
		ObjectMap m_Objects;
		UsedIdSet m_UsedIds;
		unsigned int m_ItemCounter;
	};
}