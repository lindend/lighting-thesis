#include "CrazeEngine.h"
#include "ScriptGame.h"

#include <vector>

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "Script/Script.h"
#include "Script/ScriptManager.h"
#include "Level/ScriptLevel.h"

using namespace Craze;

namespace Craze
{
	bool sgame_init(ScriptGame* pGame, const std::string &gameFile, lua_State *l);
	void sgame_update(float delta);
	void sgame_destroy();
}

ScriptGame::ScriptGame(const std::string &gameFile, lua_State *l)
{
	sgame_init(this, gameFile, l);
}

void ScriptGame::VUpdate(float delta)
{
	Game::VUpdate(delta);
	sgame_update(delta);
}
void ScriptGame::VDestroy()
{
	Game::VDestroy();
	sgame_destroy();
}

ScriptGame* pSGame;
lua_State *L = nullptr;
Script *pGameScript = nullptr;
Script *pStateScript = nullptr;
std::string statePath = "";
std::vector<IUpdateable*> updateables;
ICameraController* pCamCtrl = nullptr;
float lastFps;

bool switchState = false;
std::string newState;

bool Craze::sgame_init(ScriptGame* pGame, const std::string &file, lua_State *l)
{
	pSGame = pGame;
	L = l;
	int size;
	char *pData = gResMgr.readFileData(gFileDataLoader.addFile(file), size);
	gScriptManager.LoadScript(file, pData, size, pGameScript);

	delete [] pData;
	return (pGameScript != nullptr);
}
void Craze::sgame_destroy()
{
	delete pGameScript;
	delete pStateScript;

}
void changeState()
{
	switchState = false;

	for (auto i = updateables.begin(); i != updateables.end(); ++i)
	{
		delete (*i);
	}
	updateables.clear();

	if (pStateScript)
	{
		gScriptManager.ClearEnvironment(pStateScript->GetEnv());
	}
	delete pStateScript;

	int size;
	char *pData = gResMgr.readFileData(gFileDataLoader.addFile(statePath + "/" + newState + ".lua"), size);
	gScriptManager.LoadScript(newState, pData, size, pStateScript, pGameScript->GetEnv());
	delete [] pData;
}

void removeUpdateableAt(int idx)
{
    assert(idx < updateables.size());

    if (idx + 1 < updateables.size())
    {
        memmove(&updateables[idx], &updateables[idx + 1], sizeof(void*) * (updateables.size() - idx - 1));
        updateables.pop_back();
    } else
    {
        updateables.pop_back();
    }
}

void Craze::sgame_update(float delta)
{
	lastFps = 1.f / delta;

	if (switchState)
	{
		changeState();
	}

	if (pCamCtrl)
	{
		pCamCtrl->Update(delta);
	}

	for (int i = 0; i < (int)updateables.size();)
	{
		if (updateables[i]->VUpdate(delta))
		{
			++i;
		} else
		{
			removeUpdateableAt(i);
		}
	}
}
void Craze::sgame_setStatePath(const std::string &path)
{
	statePath = path;
}
void Craze::sgame_setState(const std::string &name)
{
	//We are clearing a lot of stuff when switching states, better do it at a safe time
	switchState = true;
	newState = name;
}
void Craze::sgame_addUpdateable(IUpdateable *pUpd)
{
	updateables.push_back(pUpd);
}
void Craze::sgame_removeUpdateable(IUpdateable *pUpd)
{
	for (int i = 0; i < (int)updateables.size(); ++i)
	{
		if (updateables[i] == pUpd)
		{
			removeUpdateableAt(i);
			return;
		}
	}
}

void Craze::sgame_loadFile(const std::string& fileName)
{
	int size;
	char *pData = gResMgr.readFileData(gFileDataLoader.addFile(fileName), size);
	gScriptManager.RunScript(fileName, pData, size, pStateScript ? pStateScript->GetEnv() : pGameScript->GetEnv());
	delete [] pData;
}

ScriptLevel* Craze::sgame_newLevel()
{
	return CrNew ScriptLevel();
}

void Craze::sgame_setLevel(ScriptLevel* pLevel)
{
	pSGame->SetLevel(pLevel);
	if (pCamCtrl)
	{
		pCamCtrl->BindCamera(pLevel->GetGraphicsScene()->getCamera());
	}
}

void Craze::sgame_setCameraController(ICameraController* pCtrl)
{
	if (pCamCtrl)
	{
		delete pCamCtrl;
	}

	pCamCtrl = pCtrl;
	if (pSGame->GetLevel())
	{
		pCamCtrl->BindCamera(pSGame->GetLevel()->GetGraphicsScene()->getCamera());
	}
}

float Craze::sgame_getfps()
{
	return lastFps;
}