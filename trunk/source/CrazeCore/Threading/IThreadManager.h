#pragma once
#include "CrazeEngine.h"

#include "IThread.h"

namespace Craze
{
	namespace Threading
	{


		class IThreadManager
		{
		public:
			virtual HTHREAD VAddThread(IThread* pThread) = 0;
			virtual void VCloseThread(HTHREAD hThread) = 0;

			virtual void VClear() = 0;

			//Used for the single threaded multitasking system...
			virtual void VUpdate(float delta) = 0;
		};
	}
}