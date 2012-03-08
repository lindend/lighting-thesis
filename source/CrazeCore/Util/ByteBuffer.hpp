#pragma once
#ifndef CRAZE__BYTE__BUFFER__H__
#define CRAZE__BYTE__BUFFER__H__

/*
Todo:
Clear

Multithreadability
Cloning / copying + operators
*/
namespace Craze
{
	namespace Util
	{
		class ByteBuffer
		{
		public:
			ByteBuffer(unsigned int bufferLen)
			{
				m_pBuffer = new char[bufferLen];
				m_Length = bufferLen;
				m_ReadHead = 0;
				m_WriteHead = 0;
				m_pCounter = new unsigned int;
				*m_pCounter = 1;
			}
			ByteBuffer(const ByteBuffer& buf)
			{
				m_DataLength = buf.m_DataLength;
				m_Length = buf.m_Length;
				m_pBuffer = buf.m_pBuffer;
				m_ReadHead = buf.m_ReadHead;
				m_WriteHead = buf.m_WriteHead;
				m_pCounter = buf.m_pCounter;
				(*m_pCounter)++;
			}
			~ByteBuffer()
			{
				(*m_pCounter)--;

				if (m_pCounter == 0)
				{
					delete [] m_pBuffer;
					delete m_pCounter;
				}
			}

			ByteBuffer& operator=(const ByteBuffer& buf)
			{
				m_DataLength = buf.m_DataLength;
				m_Length = buf.m_Length;
				m_pCounter = buf.m_pCounter;
				m_pBuffer = buf.m_pBuffer;
				m_ReadHead = buf.m_ReadHead;
				m_WriteHead = buf.m_WriteHead;
				(*m_pCounter)++;
			}
			/*
			Read
			Reads data into pOut from the ByteBuffer.
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
			/*
			Makes a new copy of the current data that the ByteBuffer has.
			*/
			ByteBuffer Clone() const
			{
				ByteBuffer buf = ByteBuffer(m_Length);
				buf.m_DataLength = m_DataLength;
				buf.m_ReadHead = m_ReadHead;
				buf.m_WriteHead = m_WriteHead;

				memcpy(buf.m_pBuffer, m_pBuffer, m_Length);

				return buf;
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
				return Peek(out, sizeof(T));
			}
			template <typename T> bool Peek(T& out, unsigned int offset) const
			{
				return Peek((char*)out, sizeof(T), offset);
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
			ByteBuffer this method will return false.
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

				return true;

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

		protected:
			char* m_pBuffer;
			unsigned int m_ReadHead;
			unsigned int m_WriteHead;
			unsigned int m_Length;
			unsigned int m_DataLength;
			unsigned int* m_pCounter;
		};
	}
}

#endif /*CRAZE__BYTE__BUFFER__H__*/