#include "IComponent.h"
#include "EventLogger.h"

using namespace Craze;
using Craze::Graphics::UI::IComponent;
using Craze::Graphics::UI::ComponentSet;

IComponent::IComponent()
{
	m_Transform = Math::Matrix4::IDENTITY;
}

void IComponent::AddChild(IComponent *pChild)
{
	m_Children.insert(pChild);
}

void IComponent::RemoveChild(Craze::Graphics::UI::IComponent *pChild)
{
	m_Children.erase(pChild);
	delete pChild;
}


void IComponent::RemoveAllChildren()
{
	for (ComponentSet::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		delete (*i);
	}

	m_Children.clear();

}
void IComponent::SetPosition(float xpos, float ypos)
{
	m_Position = Math::Vector2(xpos, ypos);
	m_Transform.SetTranslation(Math::Vector3(xpos, ypos, 0.0f));
}

void IComponent::SetSize(float width, float height)
{
	m_Size = Math::Vector2(width, height);
	m_Transform[0][0] = width;
	m_Transform[1][1] = height;
}

void IComponent::Remove()
{
	RemoveAllChildren();
	m_pParent->RemoveChild(this);

}


void IComponent::SetParent(IComponent *pParent)
{
	m_pParent = pParent;

}


void IComponent::DrawChildren(Craze::Graphics::Renderer::Renderer* pRenderer)
{
	if (VGetParams()->visible)
	{
		pRenderer->PushMatrix(m_Transform);

		VDraw(pRenderer);
		for (ComponentSet::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
		{
			(*i)->DrawChildren(pRenderer);
		}

		pRenderer->PopMatrix();
	}

}

void IComponent::UpdateChildren(float delta)
{
	VUpdate(delta);
	for (ComponentSet::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		(*i)->UpdateChildren(delta);
	}
}

IComponent* IComponent::OnClick(float xpos, float ypos)
{

	if (VIsOver(xpos, ypos))
	{
		Math::Vector2 dPos = Math::Vector2(xpos, ypos) - m_Position;
		dPos.x /= m_Size.x;
		dPos.y /= m_Size.y;

		for (ComponentSet::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
		{
			IComponent* pClicked = (*i)->OnClick(dPos.x, dPos.y);
			if (pClicked)
			{
				return pClicked;
			}
		}

		if (VGetParams()->clickable)
		{
			return this;
		}
	}

	return 0;
}