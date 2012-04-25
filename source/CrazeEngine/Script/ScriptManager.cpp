#include "CrazeEngine.h"
#include "ScriptManager.h"

#include <iostream>

#include "luabind/luabind.hpp"
extern "C"
{
#include "lua/lualib.h"
}

#include "Script.h"

const int GCFrameRateLimit = 30;
const int MinFramesBetweenGC = 10;
const int MaxFramesBetweenGC = 500;
//A factor that determines how much work should be done "in a non-specified way" (Programming in Lua).
//At least a higher value means more work, find something suitable for this
const int GCStepFactor = 1000;

using namespace Craze;

void craze_open_core(lua_State *L);
void craze_open_game(lua_State *L);
void craze_open_ui(lua_State *L);
void craze_open_graphics(lua_State *L);
void craze_open_event(lua_State* L);

namespace Craze
{
	ScriptManager gScriptManager;
}

bool ScriptManager::Initialize()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	
	luabind::open(L);

	craze_open_core(L);
	craze_open_game(L);
	craze_open_ui(L);
	craze_open_graphics(L);
	craze_open_event(L);

	//The GC might collect stuff when we don't want it to, stop it from doing so
	lua_gc(L, LUA_GCSTOP, 0);
	
	return true;
}

void ScriptManager::Shutdown()
{
	if (L)
	{
		lua_close(L);
	}
}

void ScriptManager::Update(float delta)
{
	static int FramesSinceGC = 0;
	++FramesSinceGC;
	//Check if it's time for a gc
	if ((FramesSinceGC > MinFramesBetweenGC && delta < (1.f / GCFrameRateLimit)) || FramesSinceGC > MaxFramesBetweenGC)
	{
		lua_gc(L, LUA_GCSTEP, GCStepFactor);
		FramesSinceGC = 0;
	}
}

bool ScriptManager::RunScript(const std::string& name, const char* pBuffer, int size, int env)
{
	const int res = luaL_loadbuffer(L, pBuffer, size, name.c_str());
	if (res == LUA_ERRSYNTAX)
	{
		const std::string err = lua_tostring(L, -1);
		LOG_ERROR("Syntax error in " + name + ": " + err);
		return false;
	}
	lua_rawgeti(L, LUA_REGISTRYINDEX, env);
	lua_setfenv(L, -2);
	try
	{
		if (lua_pcall(L, 0, 0, 0) == LUA_ERRRUN)
	    {
		    const std::string err = lua_tostring(L, -1);
		    LOG_ERROR("Error while loading lua script: " + err);
		    return false;
	    }
	} catch (luabind::error& err)
	{
		LOG_ERROR(std::string("Error while running script: ") + err.what());
		return false;
	}

	return true;
}

bool ScriptManager::LoadScript(const std::string &name, const char *pBuffer, int size, Script *&pOutScript, int env)
{

	CreateEnvironment(env);
	const int envRef = luaL_ref(L, LUA_REGISTRYINDEX);
	
	const int res = luaL_loadbuffer(L, pBuffer, size, name.c_str());
	if (res == LUA_ERRSYNTAX)
	{
		const std::string err = lua_tostring(L, -1);
		LOG_ERROR("Syntax error in " + name + ": " + err);
		return false;
	}
	lua_rawgeti(L, LUA_REGISTRYINDEX, envRef);
	lua_setfenv(L, -2);

	pOutScript = CrNew Script(L, envRef);
	
	if (lua_pcall(L, 0, 0, 0) == LUA_ERRRUN)
	{
		const std::string err = lua_tostring(L, -1);
		LOG_ERROR("Error while loading lua script: " + err);
		return false;
	}
	return true;
}

void ScriptManager::CreateEnvironment(int env)
{
	//This is the environment
	lua_newtable(L);

	//This is the metatable
	lua_newtable(L);
	if (env == LUA_NOREF)
	{
		lua_pushvalue(L, LUA_GLOBALSINDEX);
	} else
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, env);
	}

	lua_setfield(L, -2, "__index");

	lua_setmetatable(L, -2);
}

void ScriptManager::ClearEnvironment(int env)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, env);
	const int tbl = lua_gettop(L);
	lua_pushnil(L);
	while (lua_next(L, tbl) != 0)
	{
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_settable(L, tbl);
		lua_pushnil(L);
	}
	luaL_unref(L, LUA_REGISTRYINDEX, env);
	lua_gc(L, LUA_GCCOLLECT, 1);
	//Closures doesn't seem to get collected during first pass (I think this is due to the fact that userdata is holding refs to them), collect again
	lua_gc(L, LUA_GCCOLLECT, 1);
}

std::string ScriptManager::GetUID()
{
	static unsigned int idx = 0;
	static char buffer[20];
	_itoa_s(idx, buffer, 10);
	return std::string("craze") + buffer;
}