#include "CrazeEngine.h"
#include "Script.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

using namespace Craze;

CRAZE_POOL_ALLOC_IMPL(Script);

bool Script::LoadVar(const char* pName)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, m_EnvRef);
	lua_getfield(L, -1, pName);

	if (lua_isnil(L, -1))
	{
		lua_pop(L, 2);
		return false;
	}

	lua_replace(L, -2);
	return true;
}

Script::~Script()
{
	luaL_unref(L, LUA_REGISTRYINDEX, m_EnvRef);
}