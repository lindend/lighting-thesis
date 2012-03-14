#include "CrazeEngine.h"
#include "Level.h"

#include <sstream>

#include "Device.h"

#include "../Game/Components/GraphicsComponents.h"
#include "Pathfinding/NavigationScene.h"



using namespace Craze;

namespace Craze
{
	Level* gpCurrentLevel = nullptr;
}

Level::Level()
{
	m_ObjectIdCounter = 0;
	m_pScene = CrNew Graphics2::Scene(Graphics2::gpDevice);
	m_pNavScene = CrNew NavigationScene();
	gpCurrentLevel = this;
}

Level::~Level()
{
	VClear();
    delete m_pScene;
	delete m_pNavScene;
}


bool Level::AddObject(std::string name, GameObject* pObj)
{
	PROFILEF();

	if (name == "")
	{
		std::stringstream strm;
		strm << "Object " << m_ObjectIdCounter;
		name = strm.str();
	}

	auto it = m_Objects.find(name);
	if (it != m_Objects.end())
	{
		return false;
	}
	
	pObj->SetId(m_ObjectIdCounter);

	m_Objects.insert(make_pair(name, pObj));
	++m_ObjectIdCounter;

	return true;
}

void Level::RemoveObject(GameObject* pObj)
{
	PROFILEF();
	for (auto i = m_Objects.begin(); i != m_Objects.end(); ++i)
	{
		if (i->second == pObj)
		{
			m_Objects.erase(i);
			return;
		}
	}
}

GameObject* Level::Get(const std::string& name)
{
	PROFILEF();

	auto it = m_Objects.find(name);
	return it != m_Objects.end() ? it->second : nullptr;
}

void Level::Build()
{
	PROFILEF();
	std::shared_ptr<Graphics2::Mesh> pNavMesh;
	if (m_pNavScene->Build())
	{
		pNavMesh = m_pNavScene->CreateGraphicsMesh();
	}
	if (pNavMesh && false)
	{
		static Graphics2::ModelNode* pMesh = nullptr;
		//pMesh = m_pScene->addModel(pNavMesh, Graphics2::NODE_DISPLAYABLE);
	}

	m_pScene->build();
	
}

void Level::VUpdate(float delta)
{
	PROFILE("Level::VUpdate");

	for (auto i = m_Objects.begin(); i != m_Objects.end(); ++i)
	{
		i->second->Update(delta);
	}

}

void Level::VClear()
{
	PROFILE("Level::VClear");

	for (auto i = m_Objects.begin(); i != m_Objects.end(); ++i)
	{
		delete i->second;
	}

	m_Objects.clear();
}

void Level::SetTerrain(std::shared_ptr<Terrain> pTerrain)
{
	m_pTerrain = pTerrain;
	pTerrain->AddToScene(m_pScene);
}

std::shared_ptr<Terrain> Level::GetTerrain()
{
	return m_pTerrain;
}
