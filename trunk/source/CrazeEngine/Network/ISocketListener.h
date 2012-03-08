#pragma once
#include "CrazeEngine.h"

#include "IMessage.h"


namespace Craze
{
	namespace Network
	{
		class ISocket;

		class ISocketListener
		{
		public:
			virtual void VOnMessage(IMessage* pMsg, ISocket* pSock) = 0;
			virtual void VOnDisconnect(ISocket* pSock) = 0;
		};
	}
}