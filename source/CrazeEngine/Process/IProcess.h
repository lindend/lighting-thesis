#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

namespace Craze
{
	namespace Process
	{
		typedef StringHashType ProcessType;
		class CRAZEENGINE_EXP IProcess
		{
		public:
			IProcess() { m_pNextProcess = 0; m_Dead = true; }

			virtual void VUpdate(float delta) = 0;

			virtual void VStart() { m_Dead = false; }
			virtual void VKill() { m_Dead = true; }
			virtual bool VIsDead() { return m_Dead; }

			virtual void VAddNext(IProcess* pNext) { m_pNextProcess = pNext; }
			virtual IProcess* VGetNext() { return m_pNextProcess; }

			virtual ProcessType VGetType() const = 0;

		protected:

			bool m_Dead;
			IProcess* m_pNextProcess;

		};
	}
}