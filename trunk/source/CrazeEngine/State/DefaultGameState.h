#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "IGameState.h"
namespace Craze
{
	/**
	The default game state, doesn't really do anything special.
	*/
	class CRAZEENGINE_EXP DefaultGameState : public IGameState
	{
	public:
		DefaultGameState(Game* pGame = 0) : IGameState(pGame) {}
		virtual bool VInitialize() { return true; }
		virtual bool VUpdate(float delta) { return true; }

		virtual void VOnStart() {}
		virtual void VOnStop() {}
		virtual void VOnResume() {}
		virtual void VOnPause(IGameState* newGameState) {}

		virtual void VDestroy() { }
	};
}