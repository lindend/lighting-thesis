#pragma once
#include "Level.h"

#include "luabind/luabind.hpp"

#include "Memory/MemoryManager.h"

#include "Pathfinding/Path.h"

namespace Craze
{
	class ScriptPath : public Path
	{
		CRAZE_ALLOC();
	public:
		Vec3 GetNode(int idx)
		{
			if (idx > GetNumVerts() || idx < 1)
			{
				Vec3 v = {0.f, 0.f, 0.f};
				return v;
			}
			return GetVerts()[idx - 1];
		}
		int GetNumNodes()
		{
			return GetNumVerts();
		}
	};

	class ScriptLevel : public Level
	{
		CRAZE_ALLOC();
	public:
		void AddObject(lua_State* L, const std::string& name, const luabind::object& obj);
		ScriptPath* FindPath(const Vec3& start, const Vec3& end);
		void RayCast(lua_State* L, const Vec3& pos, const Vec3& dir, float distance);
	};
}