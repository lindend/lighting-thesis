#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

namespace Craze
{
	/**
	An object manager used to manage objects with a string identifier. This manager
	is a base for other classes that needs this functionality.
	*/
	template<typename T> class CRAZEENGINE_EXP StringNameManager
	{
		typedef std::map<std::string, T*> ObjectMap;
	public:
		/**
		Adds an object to the manager. The memory of pObject must
		be allocated with the "new" keyword.
		@param name The name that the object should be linked to.
		@param pObject The pointer to the object to be stored.
		@return True if the object successfully was added to the
		collection, false if an error occured (if the name already
		existed for example).
		*/
		virtual bool VAddObject(std::string name, T* pObject);

		/**
		Removes an item from the manager and frees the memory using
		the delete keyword.
		@param name The name of the item to remove.
		@return True if the item was removed, false if
		the item was not found.
		*/
		virtual bool VRemoveItem(std::string name);

		/**
		Returns an item from the manager.
		@param name The name of the item to get.
		@return Pointer to the stored item or NULL if the
		item was not found.
		*/
		virtual T* VGetItem(std::string name);

		/**
		Clears all the objects in the list and frees the memory.
		*/
		virtual void VClear();
	};
}