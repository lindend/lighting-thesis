#pragma once

#include <functional>

#include "luabind/luabind.hpp"

namespace Craze
{
	template <typename T> void LuaPush(lua_State* L, T value)
	{
		luabind::object obj(L, value);
		obj.push(L);
	}

	class _ScriptFunctorProxyBase
	{
	public:
		virtual ~_ScriptFunctorProxyBase() 
		{
			luaL_unref(L, LUA_REGISTRYINDEX, m_FunRef);
		}
	protected:
		_ScriptFunctorProxyBase(lua_State* L, int ref) : L(L), m_FunRef(ref) {}

		void Call(int numArgs)
		{
			lua_call(L, numArgs, 0);
		}
		void PushFunct()
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, m_FunRef);
		}

		lua_State* L;
		int m_FunRef;
	};

	template<typename T0> class ScriptFunctorProxy1 : public _ScriptFunctorProxyBase
	{
	public:
		ScriptFunctorProxy1(lua_State* L, int ref) : _ScriptFunctorProxyBase(L, ref) {}
		void Callback(T0 a0)
		{
			PushFunct();
			LuaPush(L, a0);
			Call(1);
		}
		std::function<void(T0)> GetCallback()
		{
			return std::bind(&ScriptFunctorProxy1<T0>::Callback, this, std::placeholders::_1);
		}
	};
	template<typename T0, typename T1> class ScriptFunctorProxy2 : public _ScriptFunctorProxyBase
	{
	public:
		ScriptFunctorProxy2(lua_State* L, int ref) : _ScriptFunctorProxyBase(L, ref) {}
		void Callback(T0 a0, T1 a1)
		{
			PushFunct();
			LuaPush(L, a0);
			LuaPush(L, a1);
			Call(2);
		}
		std::function<void(T0, T1)> GetCallback()
		{
			return std::bind(&ScriptFunctorProxy2<T0, T1>::Callback, this, std::placeholders::_1, std::placeholders::_2);
		}
	};
	template<typename T0, typename T1, typename T2> class ScriptFunctorProxy3 : public _ScriptFunctorProxyBase
	{
	public:
		ScriptFunctorProxy3(lua_State* L, int ref) : _ScriptFunctorProxyBase(L, ref) {}
		void Callback(T0 a0, T1 a1, T2 a2)
		{
			PushFunct();
			LuaPush(L, a0);
			LuaPush(L, a1);
			LuaPush(L, a2);
			Call(3);
		}
		std::function<void(T0, T1, T2)> GetCallback()
		{
			return std::bind(&ScriptFunctorProxy3<T0, T1, T2>::Callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}
	};
	template<typename T0, typename T1, typename T2, typename T3> class ScriptFunctorProxy4 : public _ScriptFunctorProxyBase
	{
	public:
		ScriptFunctorProxy4(lua_State* L, int ref) : _ScriptFunctorProxyBase(L, ref) {}
		void Callback(T0 a0, T1 a1, T2 a2, T3 a3)
		{
			PushFunct();
			LuaPush(L, a0);
			LuaPush(L, a1);
			LuaPush(L, a2);
			LuaPush(L, a3);
			Call(4);
		}
		std::function<void(T0, T1, T2, T3)> GetCallback()
		{
			return std::bind(&ScriptFunctorProxy4<T0, T1, T2, T3>::Callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		}
	};
}