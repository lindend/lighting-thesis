#include "CrazeGfxController.h"
#include "Profiling/Profiler.h"

using Craze::Controller::CrazeGfxController;

CrazeGfxController::CrazeGfxController(Craze::Graphics::GraphicsEngine* pGfxEngine)
{
	m_pGraphicsEngine = pGfxEngine;
}

bool CrazeGfxController::VInitialize(Craze::Event::EventManager *pEventManager)
{

	m_pEventManager = pEventManager;


	return true; 
}

void CrazeGfxController::VShutdown()
{
	m_pGraphicsEngine->VShutdown();

	delete m_pGraphicsEngine;
}

bool CrazeGfxController::VUpdate(float delta)
{
	PROFILE("CrazeGfxController::VUpdate");
	m_pGraphicsEngine->VUpdate(delta);

	if (!m_pGraphicsEngine->VRenderScene())
	{
		//Do something?!
		return false;
	}

	m_pGraphicsEngine->VPresent();
	

	return true;
}

void CrazeGfxController::VOnEvent(Craze::Event::IEvent* pEvent)
{
	//Parse all possible events here :]
}