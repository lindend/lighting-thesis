#pragma once
#include "CrazeEngine.h"

#include "ISocket.h"

#include "winsock.h"

namespace Craze
{
	namespace Network
	{
		class Win32Socket : public ISocket
		{
		protected:
			SOCKET m_Socket;
		public:
			Win32Socket(INetworkDriver* pDriver) : ISocket(pDriver) { m_Socket = INVALID_SOCKET; }

			SOCKET GetSocket() const { return m_Socket; }
			
			virtual bool VIsConnected() { return m_Socket != INVALID_SOCKET; }

			virtual bool VSend(IMessage* pMsg);
			virtual bool VConnect(const std::string& host, unsigned short port);
			virtual bool VConnect(unsigned long host, unsigned short port);
			virtual void VDisconnect();

			virtual void VUpdate();

			virtual ~Win32Socket() { }
		};
	}
}
