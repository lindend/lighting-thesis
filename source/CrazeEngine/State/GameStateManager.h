#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "IGameState.h"

namespace Craze
{

	typedef std::map<std::string, IGameState* > GameStateList;

	/*
	GameStateManager
	Manages all the game states you add, updating the one which is set as
	currently active.
	*/
	class CRAZEENGINE_EXP GameStateManager
	{
	protected:
		//A map with all the game states, using std::string as a key
		//at the moment, should change that.
		GameStateList m_GameStates;

		std::stack<IGameState*> m_StateStack;

		Game* m_pGame;

		//Initialized flag
		bool m_Initialized;
	public:
		GameStateManager(Game* pGame) { m_pGame = pGame; m_Initialized = false; }

		bool Initialize();

		//Creates a game state and adds it to the list of available game states.
		template <class T> T* CreateGameState(std::string gameStateName)
		{
			if (m_GameStates.find(gameStateName) != m_GameStates.end())
			{
				return (T*)m_GameStates[gameStateName];
			}

			T* pGs = new T(m_pGame);

			m_GameStates[gameStateName] = pGs;

			pGs->VInitialize();

			return pGs;
		}

		/**
		Pushes a game state to the stack, only the game state at the top of the
		stack will receive update calls.
		@param name The name of the game state.
		@return The game state which was pushed.
		*/
		IGameState* PushGameState(std::string name);

		/**
		Pops a game state from the stack.
		@return Pointer to the new game state.
		*/
		IGameState* PopGameState();

		//Returns a pointer to the active game state
		IGameState* GetCurrentGameState();

		//This method should be called once a frame to make sure that
		//the active game state is properly updated.
		void OnUpdate(float delta);

		void Destroy();

	};
}