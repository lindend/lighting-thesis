#pragma once
#include "CrazeEngine.h"

#include "Util/ByteBuffer.hpp"

/*
Ignore this class
It's replaced (or should be) with the IEvent class instead.
*/

namespace Craze
{
	namespace Network
	{

		//Each type of message needs a matching message type!
		class IMessageType
		{
		public:
			//Returns a new instance of a message with this type
			virtual class IMessage* VCreateMessage() = 0;
			//Returns the message size, this includes the size
			//data and id data. Size == 0 means that the message
			//is of varying size and that the length of the message
			//is the 2 bytes right after the first 4 bytes (which 
			//are the message id bytes).
			virtual unsigned int VGetSize() = 0;
			//Returns the StringHashType of the message.
			virtual StringHashType VGetType() = 0;
		};


		class MessageTypePointer
		{
		protected:
			IMessageType* m_pType;
		public:
			bool operator==(const MessageTypePointer& mp) const { return m_pType->VGetType() == mp.m_pType->VGetType(); }
			bool operator< (const MessageTypePointer& mp) const { return m_pType->VGetType().GetId() < mp.m_pType->VGetType().GetId(); }
			bool operator> (const MessageTypePointer& mp) const { return m_pType->VGetType().GetId() > mp.m_pType->VGetType().GetId(); }

			IMessageType* Get() { return m_pType; }
			void Set(IMessageType* pMT) { m_pType = pMT; }

			void Destroy() { delete m_pType; }

			MessageTypePointer(IMessageType* pMT) { m_pType = pMT; }
		};

		class IMessage
		{
		public:

			virtual Util::ByteBuffer VSerialize() const = 0;
			virtual bool VDeserialize(Util::ByteBuffer pData) = 0;

		};
	}
}
