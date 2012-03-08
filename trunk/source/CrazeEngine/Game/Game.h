#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "Application/Application.h"
#include "State/GameStateManager.h"
#include "Scene/Scene.h"
#include "Process/ProcessManager.h"
#include "Level/Map.h"

namespace Craze
{
	class Level;

    /**
    Interface for the core game logic class. This class will be responsible
    for managing game states, taking care of the logic and such.
    */
    class CRAZEENGINE_EXP Game
    {
    public:
        Game();

        /**
        Updates the game, should be called once per frame.
        Input: Time in seconds since last frame
        */
        virtual void VUpdate(float delta);

        /**
        Destroys the game instance and all sub systems
        */
        virtual void VDestroy();

        /**
        Changes the current game state to the specified game state
        @param gameStateName the string identifier of the game state to change to
        */
        void PushState(std::string gameStateName);

        void PopState();

        /**
        @return A pointer to the active game state
        */
        IGameState* GetActiveState() const;

        /**
        @return The game logic process manager
        */
        Process::ProcessManager* GetProcessManager();

        /**
        Initializes the game logic core
        @param pApplication a pointer to the application object used by the game
        @return Success of the initialization
        */
        virtual bool VInitialize();

        void SetMap(std::shared_ptr<Map> pMap) { m_pMap = pMap; }
        std::shared_ptr<Map> GetMap() { return m_pMap; }

		void SetLevel(Level* pLevel);

        Level* GetLevel() { return m_pActiveLevel; }

        virtual ~Game() { }

	protected:
        /**
        The games state manager, it handles shifting between different
        states of the game.
        */
        GameStateManager* m_pStateManager;

        /**
        The process manager provides a way to run small processes
        that gets updated each frame.
        */
        Process::ProcessManager* m_pProcessManager;

        /**
        The currently active level in the game. Can be 0
        */
        Level* m_pActiveLevel;

        /**
        The current map of the game
        */
        std::shared_ptr<Map> m_pMap;

        /**
        Initializes the games different states.
        @return Indicates if the initialization was successful.
        */
        virtual bool VInitGameStates() = 0;
    };

	extern CRAZEENGINE_EXP Game* gpGame;
}
