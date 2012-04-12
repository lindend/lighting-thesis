#include "CrazeEngine.h"

#include "luabind/luabind.hpp"
#include "luabind/adopt_policy.hpp"
#include "luabind/out_value_policy.hpp"

#include "CrazeGraphics/Device.h"

#include "Game/Components/ComponentFactory.h"
#include "Game/Components/CoreComponents.h"
#include "Game/GameObject.h"
#include "Game/ScriptGame.h"
#include "Level/ScriptLevel.h"
#include "Pathfinding/NavigationScene.h"

using namespace Craze;
using namespace luabind;

//Called with an array of tables consisting of all components
//Returns a table with all the created components
int createGameObject(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TSTRING);
	luaL_checktype(L, 2, LUA_TTABLE);

	//This is the table that lua will get in return
	lua_newtable(L);
		
	GameObject* pGameObj = CrNew GameObject();
	int tblLen = lua_objlen(L, 2);
	for (int i = 1; i <= tblLen; ++i)
	{
		lua_rawgeti(L, 2, i);
		luaL_checktype(L, -1, LUA_TTABLE);

		lua_getfield(L, -1, "type");
		luaL_checktype(L, -1, LUA_TSTRING);
		std::string type = lua_tostring(L, -1);
		lua_pop(L, 1);

		std::string name = type;
		if (type == "script")
		{
			lua_getfield(L, -1, "name");
			luaL_checktype(L, -1, LUA_TSTRING);
			name = lua_tostring(L, -1);
			lua_pop(L, 1);
		}

		int stackSize = lua_gettop(L);
		pGameObj->AddComponent(CreateComponent(nullptr, pGameObj, type, L));
		assert(stackSize + 1 == lua_gettop(L));
		lua_setfield(L, 3, name.c_str());
	}

	lua_settop(L, 3);
	return 1;
}

class ScriptUpdater : public IUpdateable
{
public:
	~ScriptUpdater()
	{
		sgame_removeUpdateable(this);
	}

	virtual bool VUpdate(float delta)
	{
		try
		{
			return call_function<bool>(m_Function, delta);
		}
		catch (luabind::error& err)
		{
			LOG_ERROR(std::string("ScriptUpdater, error while running script: ") + err.what() + lua_tostring(err.state(), -1));
			return false;
		}
		catch (std::exception& err)
		{
			LOG_ERROR(std::string("ScriptUpdater, error while running script: ") + err.what());// + lua_tostring(err.state(), -1));
			return false;
		}
	}

	object m_Function;
};

ScriptUpdater* beginUpdate(object const& f)
{
	if (luabind::type(f) == LUA_TFUNCTION)
	{
		ScriptUpdater* pUpd = new ScriptUpdater();
		pUpd->m_Function = f;
		sgame_addUpdateable(pUpd);
		return pUpd;
	}
	luaL_error(f.interpreter(), "Argument to beginUpdate must be of type function");
}

void stopUpdate(ScriptUpdater* pUpd)
{
	sgame_removeUpdateable(pUpd);
}

Vector2 getResolution()
{
	return Graphics2::gpDevice->GetViewPort();
}

void craze_open_game(lua_State* L)
{
	module(L, "game")
	[
		def("shutDown", &Application::Shutdown),
		def("setStatePath", &sgame_setStatePath),
		def("setState", &sgame_setState),

		class_<ScriptUpdater>("ScriptUpdater"),
		def("beginUpdate", &beginUpdate),
		def("stopUpdate", &stopUpdate),

		def("getResolution", &getResolution),
		def("setLevel", &sgame_setLevel),
		def("newLevel", &sgame_newLevel, adopt(result)),
		def("loadFile", &sgame_loadFile),
		def("setCameraController", &sgame_setCameraController),
		def("getFps", &sgame_getfps),

		class_<ScriptPath>("Path")
			.def("getNode", &ScriptPath::GetNode)
			.def_readonly("numNodes", &ScriptPath::GetNumNodes),

		class_<ScriptLevel>("Level")
			.def("add", &ScriptLevel::AddObject)
			.def("build", &ScriptLevel::Build)
			.def("findPath", &ScriptLevel::FindPath, adopt(result))
			.def("rayCast", &ScriptLevel::RayCast)
			.def_readonly("scene", &ScriptLevel::GetGraphicsScene),

		class_<GameObject, GameObject*>("GameObject"),

		class_<TransformComponent>("TransformComponent")
			//.def("setPos", &TransformComponent::SetPos)
			//.def("getPos", &TransformComponent::GetPos)
			.property("pos", &TransformComponent::GetPos, &TransformComponent::SetPos)
			.def("setOrientation", &TransformComponent::SetOrientation)
		
	];

	/*lua_getfield(L, LUA_GLOBALSINDEX, "game");
	lua_pushcfunction(L, &createGameObject);
	lua_setfield(L, -2, "createObject");
	lua_pop(L, 1);*/

	luaL_dostring(L, " \
						component = {}\
						component.transform = function (params) \
							return { type=\"transform\", x=params.x or 0, y=params.y or 0, z=params.z or 0}\
						end \
						component.mesh = function (params) \
							return { type=\"mesh\", file=params.file or \"\" }\
						end\
						component.physics = function (params) \
							return { type=\"physics\", file=params.file or \"\" }\
						end\
						component.navmesh = function (params) \
							return { type = \"navmesh\", file=params.file or \"\" }\
						end");
}