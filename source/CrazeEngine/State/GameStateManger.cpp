#include "CrazeEngine.h"
#include "GameStateManager.h"
#include "DefaultGameState.h"
#include "../Game/Game.h"


using namespace Craze;

bool GameStateManager::Initialize()
{
	CreateGameState<DefaultGameState>("Craze default state");
	PushGameState("Craze default state");
	
	m_Initialized = true;

	return true;
}

void GameStateManager::OnUpdate(float delta)
{
	PROFILE("GameStateManager::OnUpdate");
	if (!m_Initialized)
		return;

	if (!m_StateStack.empty())
	{
		GetCurrentGameState()->VUpdate(delta);
	}
}

void GameStateManager::Destroy()
{
	while (!m_StateStack.empty())
	{
		PopGameState();
	}

	for (GameStateList::iterator i = m_GameStates.begin(); i != m_GameStates.end(); ++i)
	{
		i->second->VDestroy();
		delete i->second;
	}
	m_GameStates.clear();

}

IGameState* GameStateManager::PushGameState(std::string name)
{
	PROFILE("GameStateManager::PushGameState");

	GameStateList::iterator i = m_GameStates.find(name);

	if (i == m_GameStates.end())
		return NULL;

	IGameState* pGameState = (*i).second;

	if (!m_StateStack.empty())
	{
		GetCurrentGameState()->VOnPause(pGameState);
	}

	m_StateStack.push(pGameState);

	pGameState->VOnStart();

	//m_pGame->GetApplication()->GetEventManager()->Clear();

	return pGameState;

}

IGameState* GameStateManager::PopGameState()
{
	if (m_StateStack.empty())
	{
		return NULL;
	}

	GetCurrentGameState()->VOnStop();

	m_StateStack.pop();

	if (!m_StateStack.empty())
	{
		m_StateStack.top()->VOnResume();
	}

	return GetCurrentGameState();

}

IGameState* GameStateManager::GetCurrentGameState()
{
	if (m_StateStack.empty())
	{
		return NULL;
	}
	
	return m_StateStack.top();
}