#pragma once
#include "CrazeEngine.h"
#include "IRunable.h"

namespace Craze
{
	namespace Threading
	{
		//Thread handle type
		typedef unsigned int HTHREAD;

		enum FRAMERATE
		{
			FRAMERATE_UNLIMITED = 0;
		};

		class IThread
		{
		protected:
			HTHREAD m_Handle;
			unsigned int m_Framerate;
			IRunable* m_pRunable;
		public:
			IThread(IRunable* pRunable) { m_pRunable = pRunable; }

			void SetFramerate(unsigned int framerate) { m_Framerate = framerate; }
			unsigned int GetFramerate() const { return m_Framerate; }

			void SetHandle(HTHREAD hThread) { m_Handle = hThread; }
			HTHREAD GetHandle() const { return m_Handle; }

		};
	}
}
