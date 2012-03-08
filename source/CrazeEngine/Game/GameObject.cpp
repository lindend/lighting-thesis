#include "CrazeEngine.h"
#include "GameObject.h"
#include "Components/IGameComponent.h"

using namespace Craze;

CRAZE_POOL_ALLOC_IMPL(GameObject);

GameObject::~GameObject()
{
	for (auto i = m_pComponents.rbegin(); i != m_pComponents.rend(); ++i)
	{
		delete (*i);
	}
	m_pComponents.clear();
}

void GameObject::AddComponent(IGameComponent* pComponent)
{
	m_pComponents.push_back(pComponent);
}

void GameObject::Update(float delta)
{
	for (auto i = m_pComponents.begin(); i != m_pComponents.end(); ++i)
	{
		(*i)->VUpdate(delta);
	}
}