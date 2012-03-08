#include "CrazeEngine.h"
#include "PlayGameState.h"
#include "Game/Game.h"
#include "Application/Application.h"

using namespace Craze;

PlayGameState::PlayGameState(Craze::Game* pGame) : IGameState(pGame)
{
	
}

bool PlayGameState::VUpdate(float delta)
{
	m_pGame->GetLevel()->VUpdate(delta);
	return true;
}

bool PlayGameState::VInitialize()
{

	return true;
}

void PlayGameState::VOnStart()
{


}


void PlayGameState::VOnStop()
{
}

void PlayGameState::VOnResume()
{

}

void PlayGameState::VOnPause(IGameState* newGameState)
{

}

void PlayGameState::VDestroy()
{
}


PlayGameState::~PlayGameState()
{
}
