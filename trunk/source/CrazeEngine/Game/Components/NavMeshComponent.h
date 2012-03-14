#pragma once
#include "Export.h"

#include "Memory/MemoryManager.h"

#include "IGameComponent.h"

struct lua_State;
namespace Craze
{
	class Level;
	class GameObject;
	class Resource;
	class NavInputMesh;

	class CRAZEENGINE_EXP NavMeshComponent : public IGameComponent
	{
		CRAZE_POOL_ALLOC(NavMeshComponent);
	public:
		NavMeshComponent(std::shared_ptr<const Resource> pRes, GameObject* pOwner, Level* pLevel) : IGameComponent(pOwner), m_pNavMesh(nullptr), m_pRes(pRes), m_pLevel(pLevel) {}
		~NavMeshComponent();
		static NavMeshComponent* Create(Level* pLevel, GameObject* pOwner, lua_State* L);

		virtual void VUpdate(float delta);

		static std::string GetType() { return "navmesh"; }
		virtual std::string VGetType() const { return GetType(); }

	private:
		NavInputMesh* m_pNavMesh;
		std::shared_ptr<const Resource> m_pRes;
		Level* m_pLevel;
	};
}