#pragma once
#include "EngineExport.h"

#include "IGameState.h"
#include "Game/Game.h"
#include "Level/Level.h"

namespace Craze
{
	class IGameData;
	

	/**
	The play game state is the core of the game logic while the game is played.
	*/
	class CRAZEENGINE_EXP PlayGameState : public IGameState
	{
	protected:
		class IApplication* m_pApp;

		unsigned int m_SceneId;

		//IPlayer* m_pPlayer;

		//virtual IPlayer* VCreatePlayer() = 0;

		Level* m_pLevel;

	public:

		PlayGameState(Game* pGame);

		virtual bool VInitialize();
		virtual bool VUpdate(float delta);

		virtual void VDestroy();

		virtual void StartGame(IGameData* pData) = 0;

		virtual void VOnStart();
		virtual void VOnStop();
		virtual void VOnResume();
		virtual void VOnPause(IGameState* newGameState);

		//IPlayer* GetPlayer() { return m_pPlayer; }

		virtual ~PlayGameState();

	};
}
