#pragma once
#include "CrazeEngine/Game/Game.h"
#include "Scene\Scene.h"
#include "Graphics.h"
#include "CameraController/FreeFlyCamera.h"
#include "Intersection/BoundingBox.h"
#include "Event2/InputEvents.h"
#include "Font/FontManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"
#include "ResourceHandlers.h"

#include "Game/Components/CoreComponents.h"
#include "Game/Components/GraphicsComponents.h"
#include "Game/Components/PhysicsComponents.h"

#include "Util/Random.h"


using namespace Craze;
using namespace Craze::Graphics2;

class TestGame : public Craze::Game
{
public:
	void Key(KEYCODE kc, KEYSTATE ks)
	{
		if (kc == KC_F4 && ks == KS_PRESS)
		{
			Graphics2::gpGraphics->Params.ShowLAB ^= 1;
		} else if(kc == KC_F1 && ks == KS_PRESS)
		{
			Graphics2::gpGraphics->Params.DisableDirect ^= 1;
		} else if(kc == KC_F2 && ks == KS_PRESS)
		{
			Graphics2::gpGraphics->Params.DisableIndirect ^= 1;
		} else if(kc == KC_F3 && ks == KS_PRESS)
		{
			Graphics2::gpGraphics->Params.DisableAO ^= 1;
		} else if(kc == KC_F5 && ks == KS_PRESS)
		{
			Graphics2::gpGraphics->Params.BoostIndirect ^= 1;
		} else if(kc == KC_F6 && ks == KS_PRESS)
		{
			Graphics2::gpGraphics->Params.UseESM ^= 1;
		}
	}


protected:
	virtual bool VInitGameStates() 
	{ 
		Level* pLevel = CrNew Level();
		m_pScene = pLevel->GetGraphicsScene();
		m_pActiveLevel = pLevel;

		gFileDataLoader.AddResourceLocation("Library/");

		
		const Resource* pRes = gResMgr.LoadResource("cubemania4_0.crazemesh");
		pRes->GetFinishedEvent().Await();
		MeshResource* pMeshRes = pRes->GetResourceData<MeshResource>();
		m_pScene->AddMesh(pMeshRes->m_pMesh, NODE_DISPLAYABLE)->SetTransform(Matrix4::IDENTITY);

		gpGraphics->BindScene(m_pScene);
		/*GameObjData objData;
		objData.m_Components.push_back(TransformComponent::GetDefaultData());
		objData.m_Components.push_back(TriMeshComponent::GetDefaultData(L"Library/Models/cubemania2_0.crazemesh"));
		objData.m_Components.push_back(PhysicsComponent::GetDefaultData(L"Library/Physics/cubemania.hkx"));
		pLevel->AddObject(ObjectTypeManager::Create(objData), "");*/

		/*GameObject* pObj = new GameObject();
		pObj->AddComponent(TransformComponent::Create(pObj, TransformComponent::GetDefaultData()));
		pObj->AddComponent(TriMeshComponent::Create(pObj, TriMeshComponent::GetDefaultData(L"Library/Models/sponza_0.crazemesh")));
		pObj->AddComponent(MeshPhysicsComponent::Create(pObj, pLevel->GetPhysicsScene(), L"Library/Models/sponza_0.crazemesh"));
		pLevel->AddObject(pObj, "");*/
		
		/*GameObjData dynCube;
		dynCube.m_Components.push_back(TransformComponent::GetDefaultData(-3.f, 4.3f, 0.8f));
		dynCube.m_Components.push_back(TriMeshComponent::GetDefaultData(L"Library/Models/cube_0.crazemesh"));
		dynCube.m_Components.push_back(PhysicsComponent::GetDefaultData(L"Library/Physics/cube.hkx"));
		pLevel->AddObject(ObjectTypeManager::Create(dynCube), "");*/

		m_pScene->AmbientLight = Vector3::ONE * 0.1f;
		m_pScene->GetCamera()->SetPosition(Vector3(3.0f, 3.0f, -3.0f));
		m_pScene->GetCamera()->SetDirection(Vector3(-1.0f, -1.0f, 1.0f).Normalized());
		m_pScene->GetCamera()->SetUp(Vector3::UP);
		m_pScene->GetCamera()->SetProjection(PI / 4.0f, 4.0f / 3.0f, 0.1f, 1000.0f);

		m_pScene->AddLight(CreatePointLight(Vector3::ONE, 10.f, Vector3::ONE * 0.3f));

		/*for (int i = 0; i < 500; ++i)
		{
			m_pScene->AddLight(CreatePointLight(Random::GetVector3(-7.f, 7.f), Random::GetFloat(3.f, 6.f), Random::GetVector3(0.02f, 0.1f)));
		}*/

		m_pCamCtrl = new FreeFlyCamera(m_pScene->GetCamera(), 4.f);

		Keyboard::AddListener(this, MEMFN2(TestGame::Key));
		return true; 
	}

	virtual void VUpdate(float delta)
	{
		Game::VUpdate(delta);
		m_pCamCtrl->Update(delta);
	}
private:
	Craze::Graphics2::Scene* m_pScene;
	Craze::FreeFlyCamera* m_pCamCtrl;
	Face* m_pFace;

};