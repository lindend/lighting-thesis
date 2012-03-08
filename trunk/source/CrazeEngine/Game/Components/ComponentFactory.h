#pragma once

extern "C"
{
#include "lua/lua.h"
}

#include <string>

namespace Craze
{
	class IGameComponent;
	class ComponentData;
	class GameObject;
	class Level;

	//IGameComponent* CreateComponent(GameObject* pGameObj, const ComponentData& compData, lua_State* L);
	IGameComponent* CreateComponent(Level* pLevel, GameObject* pGameObj, const std::string& type, lua_State* L);
}