#include "CrazeEngine.h"
#include "Win32NetworkDriver.h"

#include "winsock.h"

Craze::Network::ISocket* Craze::Network::Win32NetworkDriver::VCreateSocket()
{
	return new Craze::Network::Win32Socket(this);
}

bool Craze::Network::Win32NetworkDriver::VInitialize()
{
	LOG_INFO("Initializing Win32 network driver");
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		LOG_CRITICAL("Error while initializing the Win32 network driver");
		return false;
	}
	
	return true;
}

void Craze::Network::Win32NetworkDriver::VShutdown()
{
	LOG_INFO("Shutting down Win32 network driver");
	WSACleanup();
}

void Craze::Network::Win32NetworkDriver::VUpdate()
{

}