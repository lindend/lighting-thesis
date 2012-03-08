#include "UserInterface.h"
#include "Container.h"
#include "Event/InputEvents.h"
#include "Profiling/Profiler.h"

#include <iostream>

using Craze::Graphics::UI::UserInterface;
using Craze::Graphics::UI::Button;
using Craze::Graphics::UI::Container;
using Craze::Graphics::UI::IComponent;
using Craze::Event::InputEvents::MouseDownEvent;
using Craze::Event::InputEvents::MouseMoveEvent;
using Craze::Event::InputEvents::KeyEvent;

UserInterface::UserInterface(Craze::Graphics::IDevice* pDevice, Craze::Graphics::GraphicsResourceManager* pResMgr, Craze::Event::EventManager* pEventManager)
{
	m_Root = CENEW Container("ROOT");
	m_pDevice = pDevice;
	m_pResMgr = pResMgr;
	m_ViewMatrix.SetToIdentity();
	m_ViewMatrix.Translate(Craze::Math::Vector3(0.0f, 0.0f, 1.0f));
	m_pEventManager = pEventManager;
	
	m_pCursor = CENEW Craze::Graphics::UI::Image(m_pDevice, "Cursor");
	m_pCursor->SetSize(30 / m_pDevice->VGetResolution().x, 30 / m_pDevice->VGetResolution().y);
	m_pCursor->SetTexture(m_pResMgr->RequestTexture("cursor.png"));
	m_pCursor->VGetParams()->level = Craze::Graphics::UI::CL_FORE;
	m_pCursor->VGetParams()->depth = 0;

	m_HookIdClick = m_pEventManager->AddHook(MouseDownEvent::m_EvtType, MEMFN(UserInterface::OnClick));
	m_HookIdMove = m_pEventManager->AddHook(MouseMoveEvent::m_EvtType, MEMFN(UserInterface::OnMove));
	m_HookKey = m_pEventManager->AddHook(KeyEvent::Type, MEMFN(UserInterface::OnKey));

	m_pKeyHook = 0;
}

UserInterface::~UserInterface()
{
	m_pEventManager->RemoveHook(MouseDownEvent::m_EvtType, m_HookIdClick);
	m_pEventManager->RemoveHook(MouseDownEvent::m_EvtType, m_HookIdMove);
}

void UserInterface::Update(float delta)
{

	m_Root->UpdateChildren(delta);

}


void UserInterface::Render(Craze::Graphics::Renderer::Renderer *pRenderer)
{
	PROFILE("UserInterface::Render");
	
	m_Root->DrawChildren(pRenderer);

	m_pCursor->DrawChildren(pRenderer);


}

bool UserInterface::OnClick(Craze::Event::IEvent* pEvent)
{
	MouseDownEvent* pEvt = pEvent->Cast<MouseDownEvent>();

	IComponent* pClicked = m_Root->OnClick(pEvt->GetPosition().x / 400 - 1, pEvt->GetPosition().y / -300 + 1);
	if (!pClicked)
	{
		return true;
	}

	return !pClicked->VHandleClick();
}

bool UserInterface::OnMove(Craze::Event::IEvent* pEvent)
{
	MouseMoveEvent* pEvt = pEvent->Cast<MouseMoveEvent>();

	Math::Vector2 mousePos = pEvt->GetPosition();
	mousePos.x /= 400;
	mousePos.y /= 300;
	mousePos.x -= 1.0f;
	mousePos.y -= 1.0f;
	mousePos.y *= -1;

	m_pCursor->SetPosition(mousePos.x, mousePos.y);

	return true;

}

bool UserInterface::OnKey(Craze::Event::EventPtr pEvent)
{
	KeyEvent* pEvt = pEvent->Cast<KeyEvent>();
	
	if (m_pKeyHook)
	{
		return m_pKeyHook->VOnKey(pEvt->GetKey());
	}

	return true;
}

Button* UserInterface::CreateButton(float xpos, float ypos, float width, float height, Craze::Graphics::UI::IComponent *pParent, std::string name)
{

	Button* pBtn = CENEW Button(m_pDevice, xpos, ypos, width, height, name);
	
	if (pParent)
	{
		pBtn->SetParent(pParent);
		pParent->AddChild(pBtn);
	} else
	{
		pBtn->SetParent(m_Root);
		m_Root->AddChild(pBtn);
	}

	pBtn->SetTexture(m_pResMgr->RequestTexture(name));
	return pBtn;

}

bool UserInterface::AddComponent(Craze::Graphics::UI::IComponent* pComponent, Craze::Graphics::UI::IComponent* pParent)
{
	if (pParent)
	{
		pComponent->SetParent(pParent);
		pParent->AddChild(pComponent);
	} else
	{
		pComponent->SetParent(m_Root);
		m_Root->AddChild(pComponent);
	}

	return true;

}

void UserInterface::SetKeyHook(IComponent* pComponent)
{
	m_pKeyHook = pComponent;
}

void UserInterface::ClearKeyHook()
{
	m_pKeyHook = 0;
}

void UserInterface::ShowCursor(bool showCursor)
{
	m_pCursor->VGetParams()->visible = showCursor;
}