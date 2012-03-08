#pragma once
#include "CrazeEngine.h"

#include "IMessage.h"
#include "ISocketListener.h"


namespace Craze
{
	namespace Network
	{
		const unsigned int BUFFER_LENGTH = 1024;

		/******************************************************
		*  ISocket class - Craze::Network::ISocket
		*  Interface for keeping track of a connection over
		*  the internet. Should be implemented for the selected
		*  network library, winsock for example.
		*******************************************************/
		class ISocket
		{
		public:
			//A map that keeps track of all message listeners
			/*
			WHY DIdN'T I USE MY SPLENDId EVENT SYSTEM!?!?!?!?!!??!
			*sigh*
			Time to rewrite some stuff :[
			(Maybe add some stuff to the event system)
			*/
			typedef std::map<unsigned int, ISocketListener*> MessageListenerList;
		public:
			unsigned int m_LastMessageId;
			unsigned int m_LastMessageSize;
			MessageListenerList m_pListeners;

		protected:
			class INetworkDriver* m_pNetworkDriver;
			Util::ByteBuffer m_Buffer;

		public:
			ISocket(INetworkDriver* pDriver);

			virtual bool VIsConnected() = 0;

			/*
			VConnect
			Connects the socket to the address in host using DNS lookup.
			*/
			virtual bool VConnect(const std::string& host, unsigned short port) = 0;
			/*
			VConnect
			Connects to the IPv4 address host.
			*/
			virtual bool VConnect(unsigned long host, unsigned short port) = 0;
			/*
			VSend
			Sends the message to the client. Also updates the connected status.
			*/
			virtual bool VSend(IMessage* pMsg) = 0;
			/*
			VDisconnect
			Disconnects the client and sets the status to disconnected.
			*/
			virtual void VDisconnect() = 0;

			/*
			VUpdate
			Receivs socket information and places it in a buffer. Updates the remaining buffer length.
			*/
			virtual void VUpdate() = 0;

			Util::ByteBuffer GetBuffer()
			{
				return m_Buffer;
			}

			virtual unsigned int VGetDataLength() const { return 0; }
		
			virtual ~ISocket() { }
		};
	}
}