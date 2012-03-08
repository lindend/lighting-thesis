#include "CrazeEngine.h"
#include "MenuGameState.h"

using namespace Craze;

MenuGameState::MenuGameState(Game* pGame) : IGameState(pGame)
{

}

bool MenuGameState::VInitialize()
{
	return true;
}

