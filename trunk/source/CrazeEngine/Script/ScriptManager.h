#pragma once

extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

#include <string>

namespace Craze
{
	class Script;

	class ScriptManager
	{
		CRAZE_ALLOC();
	public:
		ScriptManager() : L(nullptr) {}

		bool Initialize();
		void Shutdown();

		void Update(float delta);

		bool RunScript(const std::string& name, const char* pBuffer, int size, int env);
		bool LoadScript(const std::string &name, const char* pBuffer, int size, Script*& pOutScript, int env = LUA_NOREF);
		void ClearEnvironment(int env);

		lua_State* GetL() const { return L; }

		static std::string GetUID();
	private:
		void CreateEnvironment(int parent);

		lua_State* L;

		ScriptManager(const ScriptManager &);
		ScriptManager & operator=(const ScriptManager &);
	};

	extern ScriptManager gScriptManager;
}