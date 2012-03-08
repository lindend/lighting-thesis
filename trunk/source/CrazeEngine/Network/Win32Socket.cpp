#include "CrazeEngine.h"
#include "Win32Socket.h"
#include "EventLogger.h"
#include "Util/ByteStream.hpp"


bool Craze::Network::Win32Socket::VSend(Craze::Network::IMessage* pMsg)
{
	if (m_Socket == INVALID_SOCKET)
	{
		return false;
	}
	unsigned int totalSent = 0;
	int sent = 0;

	Craze::Util::ByteBuffer pBs = pMsg->VSerialize();

	//This should never happen, but better safe than sorry
	if (pBs.GetContinuousDataLen() != pBs.GetDataSize())
	{
		pBs.Defrag();
	}

	//We make at most 50 attempts at sending the data
	for (int i = 0; i < 50; ++i)
	{

		sent = send(m_Socket, pBs.GetDataPointer() + totalSent, pBs.GetDataSize() - totalSent, 0);

		if (sent == 0)
		{
			m_Socket = INVALID_SOCKET;
			return false;
		}
		totalSent += sent;

		if (totalSent >= pBs.GetDataSize())
		{
			return true;
		}
	}

	return false;
}

bool Craze::Network::Win32Socket::VConnect(unsigned long host, unsigned short port)
{
	m_Socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_Socket == INVALID_SOCKET)
	{
		LOG_ERROR("Unable to create socket");
		return false;
	}

	int trueSet = 1;
	//Disable the Nagle algorithm
	setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&trueSet, sizeof(trueSet));
	setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&trueSet, sizeof(trueSet));


	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = htonl(host);

	if (connect(m_Socket, (sockaddr*)&addr, sizeof(addr)) != 0)
	{
		LOG_ERROR("Could not connect to host");
		return false;
	}

	//Set the socket to be non blocking
	unsigned long nonblocking = 1;
	ioctlsocket(m_Socket, FIONBIO, &nonblocking);

	return true;

}

bool Craze::Network::Win32Socket::VConnect(const std::string &host, unsigned short port)
{
	hostent* pHost = gethostbyname(host.c_str());

	if (!pHost)
	{
		LOG_ERROR("Could not find the host " + host);
		return false;
	}

	sockaddr_in addr;
	memcpy(&addr, pHost->h_addr_list[0], sizeof(addr));

	return VConnect(addr.sin_addr.S_un.S_addr, port);

}

void Craze::Network::Win32Socket::VUpdate()
{
	if (!VIsConnected())
	{
		return;
	}

	unsigned int maxLen = m_Buffer.GetFreeSize();

	char* pBuf = new char[maxLen];

	unsigned int recvRet = recv(m_Socket, pBuf, maxLen, 0);

	if (recvRet == SOCKET_ERROR)
	{
		VDisconnect();
		delete [] pBuf;
		return;
	}

	if (recvRet == 0)
	{
		delete [] pBuf;
		return;
	}

	m_Buffer.Write(pBuf, recvRet);

	delete [] pBuf;
}

void Craze::Network::Win32Socket::VDisconnect()
{
	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;
}