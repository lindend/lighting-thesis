#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "IGameState.h"

namespace Craze
{

	/*
	A menu game state. This will keep track of the menu.
	*/
	class CRAZEENGINE_EXP MenuGameState : public IGameState
	{
	protected:

	public:

		MenuGameState(Game* pGame);

		virtual bool VInitialize();
		virtual bool VUpdate(float delta){return true;}

		virtual void VOnChange(Craze::IGameState* newGamestate) = 0;

		virtual void VDestroy(){}

	};
}