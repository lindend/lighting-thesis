#pragma once
#include "CrazeEngine.h"

#include "IThreadManager.h"

namespace Craze
{
	namespace Threading
	{
		/*
		SingleThreadManager
		Loops through all the threads and call the update on each of their
		runables.
		*/
		class SingleThreadManager : public IThreadManager
		{
		};
	}
}
