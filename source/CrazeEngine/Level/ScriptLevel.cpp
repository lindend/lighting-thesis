#include "CrazeEngine.h"
#include "ScriptLevel.h"

#include "Game/Components/ComponentFactory.h"
#include "Pathfinding/NavigationScene.h"

using namespace Craze;
using namespace luabind;

//Return a table that contains the field __gameObject instead. Then, each function that takes a game object
//from lua must fetch this field.
void ScriptLevel::AddObject(lua_State* L, const std::string& name, const luabind::object& params)
{
	const int ArgParam = 3;

	GameObject* pGameObj = CrNew GameObject();

	lua_newtable(L);
	const int RetTblIdx = lua_gettop(L);

	lua_pushvalue(L, -1);
	pGameObj->SetScriptRef(luaL_ref(L, LUA_REGISTRYINDEX));

	lua_pushlightuserdata(L, pGameObj);
	lua_setfield(L, -2, "__gameObject");
	
	int tblLen = lua_objlen(L, ArgParam);
	for (int i = 1; i <= tblLen; ++i)
	{
		lua_rawgeti(L, ArgParam, i);
		luaL_checktype(L, -1, LUA_TTABLE);

		lua_getfield(L, -1, "type");
		luaL_checktype(L, -1, LUA_TSTRING);
		std::string type = lua_tostring(L, -1);
		lua_pop(L, 1);

		std::string compname = type;
		if (type == "script")
		{
			lua_getfield(L, -1, "name");
			luaL_checktype(L, -1, LUA_TSTRING);
			compname = lua_tostring(L, -1);
			lua_pop(L, 1);
		}

		
		const int stackSize = lua_gettop(L);
		pGameObj->AddComponent(CreateComponent(this, pGameObj, type, L));
		assert(stackSize + 1 == lua_gettop(L));
		lua_setfield(L, RetTblIdx, compname.c_str());

		lua_pop(L, 1);		
	}
	lua_settop(L, RetTblIdx);
	Level::AddObject(name, pGameObj);
}

ScriptPath* ScriptLevel::FindPath(const Vec3& start, const Vec3& end)
{
	ScriptPath* pPath = CrNew ScriptPath();
	GetNavigationScene()->FindPath(start, end, pPath);
	return pPath;
}

void ScriptLevel::RayCast(lua_State* L, const Vec3& pos, const Vec3& dir, float distance)
{
	Ray r;
	r.m_Origin = pos;
	r.m_Dir = dir;
	Vector3 hitPos;
	void* pCastRes = NULL;

	if (pCastRes)
	{
		GameObject* pGameObj = (GameObject*)pCastRes;
		lua_rawgeti(L, LUA_REGISTRYINDEX, pGameObj->GetScriptRef());
		Vec3 outPos = hitPos;
		object(L, outPos).push(L);
	}
}