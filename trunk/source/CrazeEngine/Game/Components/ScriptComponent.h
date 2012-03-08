#pragma once
#include "IGameComponent.h"

extern "C"
{
#include "lua/lua.h"
}

#include "Script/ScriptManager.h"

namespace Craze
{
	class ScriptComponent : public IGameComponent
	{
	public:
		static ScriptComponent* Create(GameObject* pOwner, lua_State* L)
		{
			//Create a reference of the input table as return value
			lua_pushvalue(L, -1);
			//Create another reference to store in the registry
			lua_pushvalue(L, -1);
			int tblRef = luaL_ref(L, LUA_REGISTRYINDEX);

			return new ScriptComponent(pOwner, L, tblRef);
		}

		~ScriptComponent() 
		{ 
			luaL_unref(m_L, LUA_REGISTRYINDEX, m_TblRef);
		}

		virtual void VUpdate(float delta)
		{
			int top = lua_gettop(m_L);
			lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_TblRef);
			lua_getfield(m_L, -1, "update");
			
			if (lua_isfunction(m_L, -1))
			{
				lua_pushnumber(m_L, delta);
				lua_pcall(m_L, 1, 0, 0);
			}

			lua_settop(m_L, top);
		}

		static std::string GetType() { return "script"; }
		virtual std::string VGetType() const { return GetType(); }
	private:
		ScriptComponent(GameObject* pOwner, lua_State* L, int tblRef) : IGameComponent(pOwner), m_L(L), m_TblRef(tblRef) {}
		int m_TblRef;
		lua_State* m_L;
	};
}