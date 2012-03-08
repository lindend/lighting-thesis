#pragma once

#include "Memory/MemoryManager.h"

struct lua_State;

namespace Craze
{
	class Script
	{
		CRAZE_POOL_ALLOC(Script);
	public:
		Script(lua_State* l, int envRef) : m_EnvRef(envRef), L(l) {}

		bool LoadVar(const char* pName);

		int GetEnv() const { return m_EnvRef; }

		~Script();
	private:
		lua_State* L;
		int m_EnvRef;

		Script(const Script &);
		Script & operator=(const Script &);
	};
}