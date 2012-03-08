#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

namespace Craze
{
	class Game;

	/**
	The game state interface. A game state is used to keep track of the current state of the game.
	*/
	class CRAZEENGINE_EXP IGameState
	{
	protected:
		Craze::Game* m_pGame;
	public:
		IGameState(Craze::Game* pGame) { m_pGame = pGame; }

		virtual bool VInitialize() = 0;
		virtual bool VUpdate(float delta) = 0;

		virtual void VOnStart() = 0;
		virtual void VOnStop() = 0;
		virtual void VOnResume() = 0;
		virtual void VOnPause(IGameState* newGameState) = 0;

		virtual Game* VGetGame() { return m_pGame; }

		virtual void VDestroy() = 0;

		virtual ~IGameState() { }
	};
}