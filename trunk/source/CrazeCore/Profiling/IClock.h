#pragma once

namespace Craze
{
	namespace Profiling
	{
		class IClock
		{
		public:
			virtual unsigned long GetClock() = 0;
		};
	}
}