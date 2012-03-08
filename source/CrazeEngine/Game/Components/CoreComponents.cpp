#include "CrazeEngine.h"
#include "CoreComponents.h"

extern "C"
{
#include "lua/lauxlib.h"
}

#include "luabind/luabind.hpp"

using namespace Craze;

CRAZE_POOL_ALLOC_ALIGN_IMPL(TransformComponent, 16);

TransformComponent* TransformComponent::Create(GameObject* pOwner, lua_State* L)
{
	int inTbl = lua_gettop(L);

	lua_getfield(L, inTbl, "x");
	lua_getfield(L, inTbl, "y");
	lua_getfield(L, inTbl, "z");
	Vector3 pos((float)luaL_optnumber(L, -3, 0.f),
				(float)luaL_optnumber(L, -2, 0.f),
				(float)luaL_optnumber(L, -1, 0.f));
	lua_pop(L, 3);

	TransformComponent* pComponent = CrNew TransformComponent(pos, Quaternion::IDENTITY, pOwner);

	luabind::object tfmCmp(L, pComponent);
	tfmCmp.push(L);

	return pComponent;
}

ComponentData TransformComponent::GetDefaultData(float x, float y, float z)
{
	ComponentData data;
	data.m_Type = GetType();
	data.m_Params.insert(std::make_pair("x", ComponentParam(x)));
	data.m_Params.insert(std::make_pair("y", ComponentParam(y)));
	data.m_Params.insert(std::make_pair("z", ComponentParam(z)));
	return data;
}