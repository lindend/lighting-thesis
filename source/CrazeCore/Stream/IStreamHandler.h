#pragma once
#include "CrazeEngine.h"

namespace Craze
{
	class IStreamHandler
	{
	public:
		virtual void VOnData(class Stream* pStream, unsigned int length) = 0;
		virtual void VOnUnplug(Stream* pStream) = 0;
	};
}
