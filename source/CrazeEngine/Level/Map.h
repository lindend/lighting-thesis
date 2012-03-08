#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "Level.h"


namespace Craze
{
	class Game;
	class IApplication;

    class CRAZEENGINE_EXP Map
    {
    public:

        Map(Game* pGame);

		Level* GetLevel(std::string name);

        Level* AddLevel(std::string name);
        void RemoveLevel(std::string name);

        static std::shared_ptr<Map> CreateFromFile(std::string mapFile);

    private:
        std::map<std::string, Level*> m_Levels;

		Game* m_pGame;
    };
}
