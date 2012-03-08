#pragma once
#include "CrazeEngine.h"

#include "INetworkDriver.h"
#include "Win32Socket.h"

namespace Craze
{
	namespace Network
	{
		class Win32NetworkDriver : public INetworkDriver
		{
		public:
			virtual bool VInitialize();
			virtual void VShutdown();

			virtual void VUpdate();

			virtual ISocket* VCreateSocket();
		};
	}
}