#include "CrazeEngine.h"
#include "Game.h"
#include "Application/Application.h"
#include "Graphics.h"
#include "Level/Level.h"

using namespace Craze;

Game* Craze::gpGame = nullptr;

Game::Game()
{
	gpGame = this;
	m_pStateManager = 0;
	m_pProcessManager = 0;
	m_pActiveLevel = nullptr;
}

bool Game::VInitialize()
{

	m_pProcessManager = new Craze::Process::ProcessManager();

	m_pStateManager = new GameStateManager(this);
	if (!m_pStateManager->Initialize())
		return false;

	if (!VInitGameStates())
		return false;

	return true;

}

void Game::VDestroy()
{
	if (m_pProcessManager)
	{
		m_pProcessManager->VDestroy();
		delete m_pProcessManager;
		m_pProcessManager = 0;
	}

	if (m_pStateManager)
	{
		m_pStateManager->Destroy();
		delete m_pStateManager;
	}
}

void Game::VUpdate(float delta)
{
	PROFILE("Game::VUpdate");

	m_pStateManager->OnUpdate(delta);
	m_pProcessManager->VUpdate(delta);
	
	if (m_pActiveLevel)
	{
		m_pActiveLevel->VUpdate(delta);
	}
}

void Craze::Game::PushState(std::string gameStateName)
{
	m_pProcessManager->VClear();
	m_pStateManager->PushGameState(gameStateName);
}

void Craze::Game::PopState()
{
	m_pStateManager->PopGameState();
}

IGameState* Craze::Game::GetActiveState() const
{
	return m_pStateManager->GetCurrentGameState();
}

Process::ProcessManager* Craze::Game::GetProcessManager()
{
	return m_pProcessManager;
}

void Game::SetLevel(Level* pLevel)
{
    m_pActiveLevel = pLevel;
    Graphics2::gpGraphics->BindScene(pLevel->GetGraphicsScene());
}
