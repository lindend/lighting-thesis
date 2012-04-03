#pragma once
#include "Memory/MemoryManager.h"
#include "Game.h"
#include "Level/ScriptLevel.h"
#include "CameraController/ICameraController.h"

struct lua_State;

namespace Craze
{
	class IUpdateable
	{
	public:
		virtual ~IUpdateable() {}
		virtual bool VUpdate(float delta) = 0;
	};

	void sgame_setStatePath(const std::string &path);
	void sgame_setState(const std::string &name);

	void sgame_addUpdateable(IUpdateable* pUpd);
	void sgame_removeUpdateable(IUpdateable* pUpd);

	ScriptLevel* sgame_newLevel();
	void sgame_setLevel(ScriptLevel* pLevel);
	void sgame_loadFile(const std::string& fileName);

	void sgame_setCameraController(ICameraController* pCamCtrl);

	float sgame_getfps();
	class ScriptGame : public Game
	{
		CRAZE_ALLOC();
	public:
		ScriptGame(const std::string& gameFile, lua_State* l);

		virtual void VUpdate(float delta);
		virtual void VDestroy();

		void SetGameState(const std::string &name) { sgame_setState(name); }

	protected:
		virtual bool VInitGameStates() { return true; }
	};
}