#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "IProcess.h"
#include "Event/EventManager.h"

namespace Craze
{
	namespace Process
	{
		class CRAZEENGINE_EXP ProcessManager
		{
		public:
			typedef std::list<IProcess*> ProcessList;

			ProcessManager()
			{
				
			}

			virtual void VDestroy();

			virtual void VClear();

			~ProcessManager();

			virtual void VAddProcess(IProcess* pProcess);

			virtual void VUpdate(float delta);

		protected:
			ProcessList m_Processes;
		};
	}
}