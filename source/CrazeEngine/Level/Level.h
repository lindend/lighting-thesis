#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "Memory/MemoryManager.h"
#include "Game/GameObject.h"
#include "Event/EventManager.h"
#include "Terrain.h"
#include "Scene/Scene.h"

namespace Craze
{
	class NavigationScene;

	/**
	The world keeps track of all the objects currently in the game, and lists all the tiles as well.
	*/
	class CRAZEENGINE_EXP Level
	{
		CRAZE_ALLOC();
	public:
		Level();

		~Level();

		Graphics2::Scene* GetGraphicsScene() { return m_pScene; }
		NavigationScene* GetNavigationScene() { return m_pNavScene; }

		//Adds an object to the scene. The scene now owns the memory of the object and will free it when the scene is deleted or cleared.
		bool AddObject(std::string name, GameObject* pObj);

		//Removes an object from the scene. This will remove the scene's ownership of the object, and it must be freed by the caller.
		void RemoveObject(GameObject* pObj);

		GameObject* Get(const std::string& name);

		void Build();

		/**
		Sets the terrain for the world
		@param pTerrain Pointer to the terrain node.
		*/
		void SetTerrain(std::shared_ptr<Terrain> pTerrain);

		/**
		@return Pointer to the terrain node in use by the world.
		*/
		std::shared_ptr<Terrain> GetTerrain();

		/**
		Updates all the game objects in the world.
		@param delta Time in milliseconds since last update.
		*/
		virtual void VUpdate(float delta);

		/**
		Clears the world from all objects and frees the memory.
		*/
		virtual void VClear();

	protected:

		std::map<std::string, GameObject*> m_Objects;
		unsigned int m_ObjectIdCounter;

		std::shared_ptr<Terrain> m_pTerrain;

		Graphics2::Scene* m_pScene;
		NavigationScene* m_pNavScene;
	};
}
