#pragma once
#include "CrazeEngine.h"
#include "ISocket.h"

namespace Craze
{
	namespace Network
	{
		class INetworkDriver
		{
		public:
			virtual bool VInitialize() = 0;
			virtual void VShutdown() = 0;

			virtual void VUpdate() =  0;

			virtual ISocket* VCreateSocket() = 0;
		protected:
		};
	}
}
