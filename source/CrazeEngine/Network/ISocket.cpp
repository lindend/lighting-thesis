#include "CrazeEngine.h"
#include "ISocket.h"

Craze::Network::ISocket::ISocket(Craze::Network::INetworkDriver *pDriver) : m_Buffer(Craze::Network::BUFFER_LENGTH)
{
	m_pNetworkDriver = pDriver;
	m_LastMessageId = 0;
	m_LastMessageSize = 0;
}
