#include "CrazeEngine.h"
#include "GraphicsComponents.h"

#include "Resource/FileDataLoader.h"

using namespace Craze;

CRAZE_POOL_ALLOC_IMPL(TriMeshComponent);

TriMeshComponent::TriMeshComponent(TransformComponent* pTfmComp, std::shared_ptr<const Resource> pRes, Level* pLevel, GameObject* pOwner) 
	: IGameComponent(pOwner), m_pTfmComp(pTfmComp), m_model(nullptr), m_pLevel(pLevel)
{ 
	m_modelRes = std::dynamic_pointer_cast<const Graphics2::Model>(pRes);
	pTfmComp->AddListener(this); 
}

TriMeshComponent::~TriMeshComponent()
{ 
	m_pTfmComp->RemoveListener(this); 
	m_pLevel->GetGraphicsScene()->removeModel(m_model);
}

void TriMeshComponent::VUpdate(float delta) 
{ 
	if (!m_model && m_modelRes && m_modelRes->getStatus() == Resource::FINISHED) 
	{ 
		m_model = m_pLevel->GetGraphicsScene()->addModel(m_modelRes, Graphics2::NODE_DISPLAYABLE);
		m_model->setTransform(m_pTfmComp->GetTransform());
	}
}

TriMeshComponent* TriMeshComponent::Create(Level* pLevel, GameObject* pOwner, lua_State* L)
{
	TransformComponent* pTfmCmp = pOwner->GetComponent<TransformComponent>();
	if (!pTfmCmp)
	{
		return nullptr;
	}

	lua_getfield(L, -1, "file");
	const char* fileName = luaL_optstring(L, -1, "");
	lua_pop(L, 1);

	std::shared_ptr<const Resource> pRes = gResMgr.loadResource(gFileDataLoader.addFile(fileName));

	lua_pushboolean(L, 1);

	return CrNew TriMeshComponent(pTfmCmp, pRes, pLevel, pOwner);
}