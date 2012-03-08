#include "CrazeEngine.h"
#include "ProcessManager.h"

using Craze::Process::ProcessManager;

void ProcessManager::VAddProcess(Craze::Process::IProcess *pProcess)
{
	PROFILE("ProcessManager::VAddProcess");
	if (!pProcess)
		return;

	m_Processes.push_back(pProcess);
	pProcess->VStart();

}


void ProcessManager::VUpdate(float delta)
{
	PROFILE("ProcessManager::VUpdate");
	std::list<ProcessList::iterator> deadProcs;

	for (ProcessList::iterator i = m_Processes.begin(); i != m_Processes.end(); ++i)
	{
		if ((*i)->VIsDead())
		{
			if ((*i)->VGetNext())
			{
				VAddProcess((*i)->VGetNext());
			}
			
			deadProcs.push_back(i);
			
		} else
		{
			(*i)->VUpdate(delta);

		}
	}

	for (std::list<ProcessList::iterator>::iterator i = deadProcs.begin(); i != deadProcs.end(); ++i)
	{

		delete (*(*i));
		m_Processes.erase((*i));
	}
}

void ProcessManager::VDestroy()
{
	for (ProcessList::iterator i = m_Processes.begin(); i != m_Processes.end(); ++i)
	{
		IProcess* p;
		p = (*i)->VGetNext();
		(*i)->VAddNext(0);

		while (p)
		{
			IProcess* n;
			n = p->VGetNext();
			p->VAddNext(0);
			delete p;
			p = n;
		}

		(*i)->VKill();
		delete (*i);
	}

	m_Processes.clear();
}

void ProcessManager::VClear()
{
	VDestroy();
}

ProcessManager::~ProcessManager()
{

}