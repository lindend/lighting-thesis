#include "CrazeEngine.h"
#include "ComponentFactory.h"

#include "IGameComponent.h"
#include "CoreComponents.h"
#include "GraphicsComponents.h"
#include "NavMeshComponent.h"
#include "ScriptComponent.h"

using namespace Craze;
/*
IGameComponent* Craze::CreateComponent(GameObject* pGameObj, const ComponentData& data, lua_State* L)
{
	IGameComponent* pCmp;
	if (data.m_Type == "script")
	{
		return ScriptComponent::Create(pGameObj, data, L);
	} else if(data.m_Type == TransformComponent::GetType())
	{
		return TransformComponent::Create(pGameObj, data, L);
	} else if(data.m_Type == TriMeshComponent::GetType())
	{
		return TriMeshComponent::Create(pGameObj, data, L);
	} else if(data.m_Type == PhysicsComponent::GetType())
	{
		return PhysicsComponent::Create(pGameObj, data, L);
	}

	return nullptr;
}*/

IGameComponent* Craze::CreateComponent(Level* pLevel, GameObject* pGameObj, const std::string& type, lua_State* L)
{
	if (type == ScriptComponent::GetType())
	{
		return ScriptComponent::Create(pGameObj, L);
	} else if(type  == TransformComponent::GetType())
	{
		return TransformComponent::Create(pGameObj, L);
	} else if(type == TriMeshComponent::GetType())
	{
		return TriMeshComponent::Create(pLevel, pGameObj, L);
	} else if(type == NavMeshComponent::GetType())
	{
		return NavMeshComponent::Create(pLevel, pGameObj, L);
	}
	return nullptr;
}