#pragma once
#include "CrazeEngine.h"

namespace Craze
{
	namespace Threading
	{
		class IRunable
		{
		public:
			/*
			VUpdate
			Called at a set interval of the thread.
			NOTE: VUpdate MUST return, it may not enter
			an infinite loop. This would cause the
			game to freeze if the SingleThreadManager
			would be in use, and we certainly wouldn't
			want that.
			*/
			virtual void VUpdate(float delta) = 0;
		};
	}
}