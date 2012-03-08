#include "CrazeEngine.h"
#include "Map.h"
#include "Game/Game.h"
#include "Application/Application.h"
#include "Graphics.h"

using namespace Craze;

Map::Map(Game* pGame) : m_pGame(pGame)
{
	
}

Level* Map::GetLevel(std::string name)
{
	auto i = m_Levels.find(name);

	if (i != m_Levels.end())
	{
		return i->second;
	}

	return nullptr;
}

Level* Map::AddLevel(std::string name)
{
	Level* pLevel = CrNew Level();
	m_Levels.insert(make_pair(name, pLevel));

	return pLevel;
}