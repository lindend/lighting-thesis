#pragma once
#include "CrazeEngine.h"

#include "IStreamHandler.h"
#include "BaseManager/BaseObject.h"

namespace Craze
{

	/*
	The Stream provides a stream interface where certain parts of the engine can write stuff and other
	parts read stuff. The readers are notified when data is written and can then act.*/
	class Stream : public BaseObject
	{
	private:
		Stream(const Stream& buf)
		{
		}
	public:
		Stream(unsigned int bufferLen, IStreamHandler* pDefaultHandler)
		{
			m_pBuffer = new char[bufferLen];
			m_Length = bufferLen;
			m_ReadHead = 0;
			m_WriteHead = 0;
			m_pCurrentHandler = 0;
			m_pDefaultHandler = pDefaultHandler;
		}

		virtual bool VUpdate()
		{
			if (m_DataLength > 0)
			{
				if (m_pCurrentHandler)
				{
					m_pCurrentHandler->VOnData(this, m_DataLength);
				}else
				{
					//Should never fail, but you never know...
					if (m_pDefaultHandler)
					{
						m_pDefaultHandler->VOnData(this, m_DataLength);
					}
				}
			}
		}

		virtual void VPlugInHandler(IStreamHandler* pHandler)
		{
			if (pHandler)
			{
				if (m_pCurrentHandler)
				{
					m_pCurrentHandler->VOnUnplug(this);
				}
				m_pCurrentHandler = pHandler;
			}
		}

		virtual void VUnplugHandler(bool notify = false)
		{
			if (notify)
			{
				if (m_pCurrentHandler)
				{
					m_pCurrentHandler->VOnUnplug(this);
				}
			}
			m_pCurrentHandler = 0;
		}

		void SetDefaultHandler(IStreamHandler* pDefaultHandler)
		{
			if (pDefaultHandler)
			{
				if (m_pDefaultHandler)
				{
					m_pDefaultHandler->VOnUnplug(this);
				}

				m_pDefaultHandler = pDefaultHandler;
			}
		}

		bool HasHandler() const
		{
			return m_pCurrentHandler != 0;
		}

		~Stream()
		{
			if (m_pCurrentHandler)
			{
				m_pCurrentHandler->VOnUnplug(this);
			}

			if (m_pDefaultHandler)
			{
				m_pDefaultHandler->VOnUnplug(this);
			}

			delete [] m_pBuffer;
		}
		/*
		Read
		Reads data into pOut from the Stream.
		If the desired len is less than the available
		buffer this method will return false. pOut
		is expected to be a memory chunk of at least
		size len.
		*/
		bool Read(char* pOut, unsigned int len)
		{
			if (len > m_DataLength)
			{
				return false;
			}

			//Check if we have to read over the boundries...
			if (m_ReadHead + len > m_Length)
			{
				unsigned int endLen = m_Length - m_ReadHead;
				memcpy(pOut, m_pBuffer + m_ReadHead, endLen);
				memcpy(pOut + endLen, m_pBuffer, len - endLen);
				m_ReadHead = len - endLen;
			} else
			{
				memcpy(pOut, m_pBuffer + m_ReadHead, len);
				m_ReadHead += len;
				if (m_ReadHead == m_Length - 1)
				{
					m_ReadHead = 0;
				}
			}

			m_DataLength -= len;

			if (m_DataLength == 0)
			{
				m_ReadHead = 0;
			}

			return true;
		}

		template <typename T> bool Read(T& out)
		{
			return Read(&out, sizeof(T));
		}

		bool Peek(char* pOut, unsigned int len) const
		{
			if (len > m_DataLength)
			{
				return false;
			}

			//Check if we have to read over the boundries...
			if (m_ReadHead + len > m_Length)
			{
				unsigned int endLen = m_Length - m_ReadHead;
				memcpy(pOut, m_pBuffer + m_ReadHead, endLen);
				memcpy(pOut + endLen, m_pBuffer, len - endLen);
			} else
			{
				memcpy(pOut, m_pBuffer + m_ReadHead, len);
			}

			return true;

		}
		bool Seek(unsigned int len)
		{
			if (len > m_DataLength)
			{
				return false;
			}
				
			m_DataLength -= len;
			m_ReadHead += len;

		}
		bool Peek(char* pOut, unsigned int len, unsigned int offset) const
		{
			if (len + offset > m_DataLength)
			{
				return false;
			}

			//Check if we have to read over the boundries...
			if (m_ReadHead + len + offset > m_Length)
			{
				if (m_ReadHead + offset > m_Length)
				{
					unsigned int endLen = m_Length - m_ReadHead;
					memcpy(pOut, m_pBuffer + offset - endLen, len);
				} else
				{
					unsigned int endLen = m_Length - m_ReadHead;
					memcpy(pOut, m_pBuffer + m_ReadHead + offset, endLen);
					memcpy(pOut + endLen, m_pBuffer, len - endLen);
				}

			} else
			{
				memcpy(pOut, m_pBuffer + m_ReadHead + offset, len);
			}

			return true;
		}
		template <typename T> bool Peek(T& out) const
		{
			return Peek(&out, sizeof(T));
		}
		template <typename T> bool Peek(T& out, unsigned int offset) const
		{
			return Peek(&out, sizeof(T), offset);
		}

		bool Discard(unsigned int len)
		{
			if (len > m_DataLength)
			{
				return false;
			}
				
			m_ReadHead += len;

			if (m_ReadHead > m_Length)
			{
				m_ReadHead -= m_Length;
			}

			m_DataLength -= len;

			if (m_DataLength == 0)
			{
				m_ReadHead = 0;
			}

			return true;
		}
		/*
		Resize
		Resizes the allocated buffer to the new
		desired size. If this size is less than
		the data length currently contained in the
		Stream this method will return false.
		*/
		bool Resize(unsigned int newSize)
		{
			if (newSize < m_DataLength)
			{
				return false;
			}

			char* pnewBuf = new char[newSize];

			Read(pnewBuf, m_DataLength);

			m_Length = newSize;
			m_ReadHead = 0;
			m_WriteHead = newSize;
			delete [] m_pBuffer;
			m_pBuffer = pnewBuf;

		}

		bool IsEmpty() const
		{
			return m_DataLength == 0;
		}
		bool IsFull() const
		{
			return m_Length == m_DataLength;
		}
		unsigned int GetFreeSize() const
		{
			return m_Length - m_DataLength;
		}
		unsigned int GetDataSize() const
		{
			return m_DataLength;
		}

		//Makes sure that the m_ReadHead is at zero
		void Defrag()
		{
			if (m_ReadHead == 0)
			{
				return;
			}

			if (m_DataLength == 0)
			{
				return;
			}

			unsigned int len = m_DataLength;
			char* pTempBuf = new char[m_DataLength];

			//Read all data into a new buffer, set the read head to zero and then write the data into the buffer again
			Read(pTempBuf, m_DataLength);
			m_ReadHead = 0;
			Write(pTempBuf, len);
				
			delete [] pTempBuf;
		}

		//Note: Make sure to use GetContinuousDataLen() to avoid reading outside of the buffer. Use with care.
		const char* GetDataPointer() const
		{
			return m_pBuffer + m_ReadHead;
		}

		unsigned int GetContinuousDataLen() const
		{
			unsigned int remLen = m_Length - m_ReadHead;

			if (remLen > m_DataLength)
			{
				return m_DataLength;
			}

			return remLen;
		}


		bool Write(const char* const data, unsigned int len)
		{
			if (len > m_Length - m_DataLength)
			{
				Resize(m_Length + len);
			}

			if (m_WriteHead + len > m_Length)
			{
				unsigned int endLen = m_Length - m_WriteHead;
				memcpy(m_pBuffer + m_WriteHead, data, endLen);
				memcpy(m_pBuffer, data + endLen, len - endLen);
				m_WriteHead = len - endLen;
			} else
			{
				memcpy(m_pBuffer + m_WriteHead, data, len);
				m_WriteHead += len;

				if (m_WriteHead == m_Length - 1)
				{
					m_WriteHead = 0;
				}
			}

			m_DataLength += len;

			return true;
		}

		template <typename T> bool Write(const T& data)
		{
			return Write(&data, sizeof(T));
		}

	protected:
		char* m_pBuffer;
		unsigned int m_ReadHead;
		unsigned int m_WriteHead;
		unsigned int m_Length;
		unsigned int m_DataLength;

		IStreamHandler* m_pCurrentHandler;
		IStreamHandler* m_pDefaultHandler;
	};
}