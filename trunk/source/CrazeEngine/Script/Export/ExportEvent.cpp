#include "CrazeEngine.h"

#include "luabind/luabind.hpp"
#include "luabind/adopt_policy.hpp"

#include "Event2/InputEvents.h"
#include "../ScriptFunctorProxy.h"
using namespace Craze;
using namespace luabind;

template <typename T> class LuaEventListener
{
public:
	LuaEventListener(const object& obj)
	{
		Init(obj);
	}

	~LuaEventListener()
	{
		Close();
	}

	void Init(const object& obj)
	{
		L = obj.interpreter();
		obj.push(L);
		const int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		_Init(L, ref, &T::Notify);
	}

	void Close()
	{
		if (m_SFPBase)
		{
			T::RemoveListener(this);
			delete m_SFPBase;
			m_SFPBase = nullptr;
		}
	}

private:
#define REG(f) T::AddListener(this, f->GetCallback()); m_SFPBase = f
	template <typename A0> void _Init(lua_State* L, int ref, void(A0))
	{
		auto f = new ScriptFunctorProxy1<A0>(L, ref);
		REG(f);
	}
	template <typename A0, typename A1> void _Init(lua_State* L, int ref, void(A0, A1))
	{
		auto f = new ScriptFunctorProxy2<A0, A1>(L, ref);
		REG(f);
	}
	template <typename A0, typename A1, typename A2> void _Init(lua_State* L, int ref, void(A0, A1, A2))
	{
		auto f = new ScriptFunctorProxy3<A0, A1, A2>(L, ref);
		REG(f);
	}
	template <typename A0, typename A1, typename A2, typename A3> void _Init(lua_State* L, int ref, void(A0, A1, A2, A3))
	{
		auto f = new ScriptFunctorProxy4<A0, A1, A2, A3>(L, ref);
		REG(f);
		//T::AddListener(this, f->GetCallback());
		//m_SFPBase = f;
	}
#undef REG

	_ScriptFunctorProxyBase* m_SFPBase;
	lua_State* L;
};

void craze_open_event(lua_State* L)
{
//#define REGEVT(name, type) namespace_(name) [ class_<LuaEventListener<type>>("EventListener"), def("listen", &startListen<type>, adopt(result)) ]
#define REGEVT(name, type) namespace_(name) [ class_<LuaEventListener<type>>("Listener") .def(constructor<const object&>()) .def("close", &LuaEventListener<type>::Close) ]
	module(L, "event")
	[
		REGEVT("mouseMove", MouseMove),
		REGEVT("mouseButton", MouseButton),
		REGEVT("mouseScroll", MouseScroll),
		REGEVT("keyboard", Keyboard)
	];
#undef REGEVT
}