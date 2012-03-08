#include "CrazeEngine.h"
#include "NetworkManager.h"

bool Craze::Network::NetworkManager::Initialize(Craze::Network::INetworkDriver *pNetworkDriver)
{
	m_pNetworkDriver = pNetworkDriver;

	if (m_pNetworkDriver->VInitialize())
	{
		LOG_CRITICAL("Error while initializing the network driver");
		return false;
	}
	return true;
}

Craze::Network::ISocket* Craze::Network::NetworkManager::Connect(const std::string &host, unsigned short port)
{
	Craze::Network::ISocket* pSock = m_pNetworkDriver->VCreateSocket();
	if (pSock->VConnect(host, port))
	{
		m_Sockets.push_back(pSock);
		return pSock;
	}

	delete pSock;

	return NULL;
}

Craze::Network::ISocket* Craze::Network::NetworkManager::Connect(unsigned long host, unsigned short port)
{
	Craze::Network::ISocket* pSock = m_pNetworkDriver->VCreateSocket();

	if (pSock->VConnect(host, port))
	{
		m_Sockets.push_back(pSock);
		return pSock;
	}

	delete pSock;

	return NULL;
}

/*Note:
Add 0 as an invalid hash for StringHashType
*/

void Craze::Network::NetworkManager::Update()
{

	Craze::Network::SocketList::iterator i = m_Sockets.begin();

	while (i != m_Sockets.end())
	{
		if ((*i)->VIsConnected())
		{
			(*i)->VUpdate();

			while (1)
			{
				/*
				Look up the current message id
				*/
				unsigned int msgId = 0;
				//Check if we have been processing a message previously
				if ((*i)->m_LastMessageId != 0)
				{
					msgId = (*i)->m_LastMessageId;

				//See if there is a new message id available
				}else if ((*i)->GetBuffer().Peek<unsigned int>(msgId))
				{			
					(*i)->m_LastMessageId = msgId;
				} else
				{
					//No message to process for this socket...
					break;
				}

				Craze::Network::MessageTypeList::iterator type = m_pMessageTypes.find(msgId);

				//Check if the message is of a valid type...
				if (type == m_pMessageTypes.end())
				{
					//The type is invalid, discard it...
					(*i)->GetBuffer().Discard(sizeof(unsigned int));
				} else
				{
					//Find the size of the message
					unsigned int size = (*type).second->VGetSize();
					if (size == 0)
					{
						//If the message has a zero size, the size is contained in the next 4 bytes
						if (!(*i)->GetBuffer().Peek<unsigned int>(size, sizeof(unsigned int)))
						{
							(*i)->m_LastMessageSize = 0;
							break;
						}

						//We successfully read data from the buffer, but size is still zero - 
						//discard the data since it is presumably invalid.
						if (size == 0)
						{
							(*i)->GetBuffer().Discard(sizeof(unsigned int) * 2);
							break;
						}
					}

					(*i)->m_LastMessageSize = size;

					//Check if we have enough data to process
					if (size <= (*i)->GetBuffer().GetDataSize())
					{
						//Finally time to process the data! :]
						Craze::Network::IMessage* pMsg = type->second->VCreateMessage();
						(*i)->GetBuffer().Discard(sizeof(unsigned int) * 2);

						if (pMsg->VDeserialize((*i)->GetBuffer()))
						{
							
						}
						
					}
				}
			}

			++i;
		} else
		{
			i.remove();
		}
	}
}

void Craze::Network::NetworkManager::RegisterMessageType(Craze::Network::IMessageType *pMsgType)
{
	if (m_pMessageTypes.find(pMsgType->VGetType().GetId()) == m_pMessageTypes.end())
	{
		m_pMessageTypes[pMsgType->VGetType().GetId()] = pMsgType;
	} else
	{
		delete pMsgType;
	}
}

Craze::Network::NetworkManager::~NetworkManager()
{
	for (Craze::Network::MessageTypeList::iterator i = m_pMessageTypes.begin(); i != m_pMessageTypes.end(); ++i)
	{
		delete (*i).second;
	}

	m_pNetworkDriver->VShutdown();
}