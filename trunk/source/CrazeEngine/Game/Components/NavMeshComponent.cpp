#include "CrazeEngine.h"
#include "NavMeshComponent.h"

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "Level/Level.h"
#include "Pathfinding/NavigationScene.h"
#include "Pathfinding/NavInputMesh.h"
#include "GraphicsComponents.h"

#include "Model.h"
#include "Geometry/MeshData.h"

using namespace Craze;
using namespace Craze::Graphics2;

CRAZE_POOL_ALLOC_IMPL(NavMeshComponent);

void NavMeshComponent::VUpdate(float delta)
{
	if (!m_pNavMesh && m_pRes && m_pRes->getStatus() == Resource::FINISHED)
	{
		const Model* model = dynamic_cast<const Model*>(m_pRes);
		if (model && !model->getMeshes().empty())
		{
			auto meshes = model->getMeshes();
			std::shared_ptr<Graphics2::MeshData> pMeshData = meshes.front().mesh->getMeshData();
			m_pNavMesh = CrNew NavInputMesh();
			m_pNavMesh->Set((const Vec3*)pMeshData->GetPosNormalUv(), sizeof(Vertex), pMeshData->GetNumVertices(), 
								pMeshData->GetIndices(), pMeshData->GetNumIndices() / 3);

			m_pLevel->GetNavigationScene()->AddMesh(m_pNavMesh);
		}
	}
}

NavMeshComponent* NavMeshComponent::Create(Level* pLevel, GameObject* pOwner, lua_State* L)
{
	lua_getfield(L, -1, "file");
	const char* fileName = luaL_optstring(L, -1, "");
	lua_pop(L, 1);

	const Resource* pRes = gResMgr.loadResource(gFileDataLoader.addFile(fileName));

	if (pRes)
	{
		lua_pushboolean(L, 1);
		return CrNew NavMeshComponent(pRes, pOwner, pLevel);
	} 
	pRes->release();
	return nullptr;	
}