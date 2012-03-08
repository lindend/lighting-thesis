#pragma once
#include "CrazeEngine.h"

#include "INetworkDriver.h"
#include <Util/CrazeArray.hpp>
#include "Stream/StreamManager.h"

namespace Craze
{
	namespace Network
	{
		//We use a set as searching is quicker than in a standard list.
		typedef std::map<unsigned int, IMessageType*> MessageTypeList;
		typedef Craze::Array<ISocket*> SocketList;
		class NetworkManager
		{
		public:
			bool Initialize(INetworkDriver* pNetworkDriver);

			ISocket* Connect(const std::string& host, unsigned short port);
			ISocket* Connect(unsigned long host, unsigned short port);

			void Update();

			void RegisterMessageType(IMessageType* pMsgType);

			StreamManager* GetStreamManager() { return m_pStreamManager; }


			~NetworkManager();
		protected:
			MessageTypeList m_pMessageTypes;
			SocketList m_Sockets;
			INetworkDriver* m_pNetworkDriver;

			StreamManager* m_pStreamManager;

		};
	}
}