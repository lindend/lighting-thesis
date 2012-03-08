#pragma once

/*----------------------------------------------------------
ByteStream v. 1.2.0 by:
Daniel Lindén (daniel-linden@hotmail.com)

Put stuff into it, and get it out in a nice chunk. Written
to make life easier when programming the network system for
my game server.

License:
Creative Commons Attribution-Noncommercial 3.0
http://creativecommons.org/licenses/by-nc/3.0/

Older versions
Some of the older versions has been saved, but some are lost. I think.
Contact me if you are interested in getting an older version and I'll see if I can find it.

Possible improvements:
Move the settings of the behaviour from the preprocessor defines into the class, maybe like
template parameters.

Changelog:

1.2.0
-Fixed the multithreading to work correcly. Previously there was a small risk that
it could fail, but now it should be safe(r). The downside is that is now only works
on windows computers, unless you change the lockable class.
-Added dependecy to the lockable class.

1.1.1
-Added reference counting so that the data would not need to be copied.
-Added Clone method that copies the data to a new instance.

1.1.0
-Changed the bytestream to use new/delete.
-Made the bytestream resize itself by BS_RESIZE_FACTOR (30% by default) each time
the allocated memory size was reached. This should give a major performance boost
if you were writing lots of data to the bytestream and did not preallocate a size.
-Added Resize() so the user can choose the allocated memory size.

1.0.5:
Added a copy assignment operator.

New in 1.0.4:
-Data not stored in network order when using the << and >> operators anymore.l
-Some minor optimizations.

New in 1.0.3:
-You can now write and read bytes in network/host order. If you
are using NetworkRead/NetworkWrite the ByteStream makes sure the
data it got is in network order and then it returns it in host 
order. This requires the ws2_32.lib to be linked, unless
BS_NETWORKBYTES_DISABLE is defined.
-Thread safety is a bit changed. It was a tough job to keep track
of all Lock() and Unlock() everywhere in the code, so I decided
to make the ByteStream lock itself. You don't have as much control
at higher levels, but its a lot easier to use. Which was the purpose
of this class from the beginning.
-Operator overloading for iostream like usage.

New in 1.0.2:
-Thread safety (I hope so at least)
-Added the option to allocate memory on creation. This will
speed up the ByteStream and is certainly recommended.
If the data is too big to fit into the pre-allocated
memory the ByteStream will extend the memory and work as usual.
-Slight optimizations on some places
-Functions are now inlined to maximized speed. This can
be prevented by using #define BS_INLINE_DISABLE before
you include ByteStream.hpp
-Most functions now have a way to inform the user that something
went wrong. See the specification of each function for more
information.
-Voidlands edition is now the official version of ByteStream
(you might say that this is not the voidlands edition anymore).

New in 1.0.1:
-Some quite big changes to make it better fit for the voidlands
project. There were improvements throughout the whole class.

----------------------------------------------------------*/

#include "optional.hpp"
#include "lockable.hpp"

#include <string>


//The standard version of the BS_WAIT_FUNC uses win32 Sleep
#ifndef BS_WAIT_FUNC
#include <windows.h>
#define BS_WAIT_FUNC(ms) Sleep(ms)
#endif //BS_WAIT_FUNC

//The resize factor.
#ifndef BS_RESIZE_FACTOR
#define BS_RESIZE_FACTOR 1.3f
#endif /*BS_RESIZE_FACTOR*/

#ifndef BS_NETWORKBYTES_DISABLE
#include <winsock.h>
#endif /*BS_NETWORKBYTES_DISABLE*/

//Wait for how long before giving up?
//(while() cycles)
#define BS_WAIT_TIMEOUT 500000

//For how long should it wait?
#define BS_WAIT_STEP 1

#ifndef BS_LOCK_DISABLED
#define BS_LOCK Lock()
#define BS_UNLOCK Unlock()
#else //BS_LOCK_DISABLED
#define BS_LOCK
#define BS_UNLOCK
#endif //BS_LOCK_DISABLED

//Check if inlining is disabled, not really recommended
#ifdef BS_INLINE_DISABLED
#define BS_INLINE
#else //BS_INLINE_DISABLED
#define BS_INLINE inline
#endif //BS_INLINE_DISABLED


class ByteStream : public Lockable
{
public:

	//Constructors and destructor
	ByteStream() : 
		m_ReadLocation(0),
		m_DataLength(0),
		m_PreAllocSize(0)
	{
		m_ByteList = 0;
		m_pReferences = new unsigned int;
		(*m_pReferences) = 1;
	}
	//Alternate contructor
	//Pre-allocates the buffer of custom
	//size to improve performance.
	//Memory usage might increase though.
	ByteStream(unsigned int bufferSize) :
		m_ReadLocation(0),
		m_DataLength(0),
		m_PreAllocSize(bufferSize)
	{
		m_ByteList = new unsigned char[bufferSize];
		m_pReferences = new unsigned int;
		(*m_pReferences) = 1;
	}

	ByteStream(const ByteStream& bs)
	{
		(*this) = bs;
	}
	~ByteStream() 
	{ 
		--(*m_pReferences);

		if ((*m_pReferences) == 0)
		{
			delete [] m_ByteList;
			delete m_pReferences;
		}
	}

	//Write data to the ByteStream, type of your choice :)
	//Arguments:
	//data (input) constant reference to the data you want to write
	//Return value: none
	template <typename _T> BS_INLINE bool Write(const _T& data)
	{

		return Write((char*)&data, (unsigned int)sizeof(_T));
	}


	//Write data to the ByteStream
	//Arguments:
	//data (input) constant pointer to the data you want to write
	//length (input) the length of your data, in bytes
	//Return value: none
	BS_INLINE bool Write(const char* const data, unsigned int length)
	{
		BS_LOCK;

		//Check if the already allocated data is large enough
		if (m_PreAllocSize - m_DataLength < length)
		{
			Enlarge(length);
		}
		memcpy(m_ByteList + m_DataLength, data, length);
		BS_UNLOCK;
		m_DataLength += length;

		return true;
	}

	//Read data, and you can choose the type here too
	//Arguments: none
	//Return value: The data at the position of the read head, by type optional<_T>
	template <typename _T> BS_INLINE optional<_T> Read()
	{

		unsigned int size = static_cast<unsigned int>(sizeof(_T));

		if (size > (m_DataLength - m_ReadLocation))
			return optional<_T>();

		_T val;

		BS_LOCK;
		memcpy(&val, m_ByteList + m_ReadLocation, size);
		BS_UNLOCK;

		m_ReadLocation += size;
		return optional<_T>(val);


	}

	//Reads data from the ByteStream and returns a pointer
	//Arguments:
	//-Pointer to an allocated buffer.
	//-Length of the data to be read, has to be <= size of buffer!
	//Return value: A boolean indicating if the operation succeeded
	BS_INLINE bool Read(char* buffer, int length)
	{
		if (m_ReadLocation + length > m_DataLength || length <= 0)
		{
			return false;
		}
		BS_LOCK;
		memcpy(buffer, m_ByteList + m_ReadLocation, length);
		m_ReadLocation += length;
		BS_UNLOCK;
		return true;

	}

	/*
	Resize
	Arguments: The new available length of the datastream.
	Return: True if the bytestream was resized, otherwise false.
	*/
	BS_INLINE bool Resize(unsigned int length)
	{
		if (length < m_DataLength)
		{
			return false;
		}

		if (length < m_PreAllocSize)
		{
			return false;
		}

		m_PreAllocSize = length;

		if (m_ByteList)
		{
			unsigned char* tmpList = new unsigned char[length];
			memcpy(tmpList, m_ByteList, m_DataLength);
			delete m_ByteList;
			m_ByteList = tmpList;
		} else
		{
			m_ByteList = new unsigned char[length];	
		}

		return true;
	}

	/*
	Clone
	Copies the ByteStream and its data to a new instance.
	Return: The cloned ByteStream.
	*/
	BS_INLINE ByteStream Clone()
	{
		ByteStream bs;
		bs.m_PreAllocSize = m_PreAllocSize;
		bs.m_ReadLocation = m_ReadLocation;
		bs.SetData((char*)m_ByteList, m_DataLength);

		return bs;
	}

//If you don't want to link in winsock
#ifndef BS_NETWORKBYTES_DISABLE

	//Read and write bytes in network and host order.
	//Works in the same way as the normal Read, just
	//it returns a predecided data type.
	BS_INLINE optional<unsigned long> NetworkReadLong()
	{
		optional<unsigned long> ret = Read<unsigned long>();
		if (ret.Valid())
		{
			ret = ntohl(ret.Get());
		}
		return ret;
	}
	BS_INLINE optional<unsigned short> NetworkReadShort()
	{
		optional<unsigned short> ret = Read<unsigned short>();
		if (ret.Valid())
		{
			ret = ntohs(ret.Get());
		}
		return ret;
	}
	BS_INLINE bool NetworkWriteLong(unsigned long val)
	{
		val = htonl(val);
		return Write<unsigned long>(val);
	}
	BS_INLINE bool NetworkWriteShort(unsigned short val)
	{
		val = htons(val);
		return Write<unsigned short>(val);
	}
#endif /*BS_NETWORKBYTES_DISABLE*/

	//Get the length of the stream
	//Arguments: none
	//Return value: Length of the ByteStream, in bytes
	BS_INLINE unsigned int GetLength() const { return m_DataLength; }

	//Get the remaining data length
	//Arguments: none
	//Return value: Number of bytes left to be read in the ByteStream
	BS_INLINE unsigned int GetRemainingDataLength() 
	{
		return m_DataLength - m_ReadLocation;
	}


	//Fetch the stored data
	//Arguments: none
	//Return value: Const pointer to the data in the bytestream
	BS_INLINE const char * GetData() const
	{ 
		return (const char*)m_ByteList;
	}

	//Set the ByteStream data, WARNING: Deletes all current data!!!
	//Arguments:
	//data (input) const pointer to the data you want to write
	//length (input) length of the data you want to write
	//Return value: none
	BS_INLINE bool SetData(const char * data, unsigned int length)
	{

		//This check should not be necessary, but I keep it since it doesn't
		//slow anything down noticable.
		if (!length || !data)
		{
			return false;
		}

		BS_LOCK;
		if (m_PreAllocSize < length)
		{
			Resize(length);
		}

		m_DataLength = length;

		memcpy(m_ByteList, data, length);
		BS_UNLOCK;

		return true;
	}


	//Deletes a number of bytes from the end of the stream
	//Arguments: size to remove at the end
	//Return value: A boolean indicating if the operation succeeded
	BS_INLINE bool RemoveAtEnd(unsigned int length)
	{
		if (length <= m_DataLength)
		{
			BS_LOCK;

			m_DataLength -= length;

			BS_UNLOCK;
			return true;
		}

		return false;		

	}

	//Deletes a number of bytes in a specific place of the stream
	//Arguments:
	//-at what index to begin removal
	//-size of the data to remove
	//Return value: A boolean indicating if the operation succeeded
	BS_INLINE bool RemoveAt(unsigned int index, unsigned int length)
	{


		if (length <= m_DataLength)
		{
			BS_LOCK;
			if (cpy(index + length, index, m_DataLength - length - index))
			{
				RemoveAtEnd(length);
				BS_UNLOCK;
				return true;
			}
			BS_UNLOCK;
		}
		return false;

	}

	//Encrypt the data using the Tiny Encryption Algorithm, note that this function enlarges the data so it can
	//			be divided by 8. This is necessary as the encryption algorithm takes 8 bytes at the time.
	//Arguments:
	//key (input) an array with 4 elements of unsigned long, they key for the encryption
	//Return value: A boolean indicating if the operation succeeded (always does if the ByteStream is locked)
	BS_INLINE bool Encrypt(unsigned long const key[4])
	{
		BS_LOCK;

		int addSize =  8 - (m_DataLength % 8);

		if (addSize == 8)
			addSize = 0;

		m_DataLength = m_DataLength + addSize;

		if (addSize)
		{
			if (m_PreAllocSize < m_DataLength)
			{
				Resize(m_DataLength);
			}
		}

		for (int i = 0; i < (int)(m_DataLength / 8); i++)
		{
			encipher((unsigned long*)(m_ByteList + 8 * i), (unsigned long*)(m_ByteList + 8 * i), key);
		}
		BS_UNLOCK;
		return true;

	}

	//Decrypt the data using the Tiny Encryption Algorithm, the data in the bytestream has to be divideable with 8
	//Arguments:
	//key (input) an array with 4 elements of unsigned long, they key for decryption. 
	//			Has to be exactly the same as the key used in Encrypt or data will be corrupted.
	//Return value: A boolean indicating if the operation succeeded
	BS_INLINE bool Decrypt(unsigned long const key[4])
	{
		BS_LOCK;

		if (m_DataLength % 8 != 0)
			return false;

		for (int i = 0; i < (int)(m_DataLength / 8); i++)
		{
			decipher((unsigned long*)(m_ByteList + 8 * i), (unsigned long*)(m_ByteList + 8 * i), key);
		}
		BS_UNLOCK;
		return true;


	}

	//Set where you want to read
	//Arguments:
	//location (input) set where in the bytestream you want the read head
	//Return value: none
	BS_INLINE void SetReadHead(unsigned int location)
	{
		BS_LOCK;

		if (location < 0)
		{
			m_ReadLocation = 0;
			BS_UNLOCK;
			return; 
		}

		if (location > m_DataLength)
		{
			m_ReadLocation = m_DataLength;
			BS_UNLOCK;
			return; 
		}

		m_ReadLocation = location;

		BS_UNLOCK;
	}

	const ByteStream& operator= (const ByteStream& bs)
	{
		m_PreAllocSize = bs.m_PreAllocSize;
		m_ReadLocation = bs.m_ReadLocation;
		m_ByteList = bs.m_ByteList;
		m_pReferences = bs.m_pReferences;
		++(*m_pReferences);

		return *this;
	}

	ByteStream& operator<< (const int val)
	{
		Write<int>(val);
		return *this;
	}
	ByteStream& operator<< (const long val)
	{
		Write<long>(val);
		return *this;
	}
	ByteStream& operator<< (const float val)
	{
		Write<float>(val);
		return *this;
	}
	ByteStream& operator<< (const std::string& val)
	{
		Write(val.c_str(), (unsigned int)val.length());
		return *this;
	}
	ByteStream& operator<< (const short val)
	{
		Write<short>(val);
		return *this;
	}
	ByteStream& operator<< (const char val)
	{
		Write<char>(val);
		return *this;
	}
	ByteStream& operator>> (int& val)
	{
		val = Read<int>().Get();
		return *this;
	}
	ByteStream& operator>> (float& val)
	{
		val = Read<float>().Get();
		return *this;
	}
	ByteStream& operator>> (long& val)
	{
		val = Read<long>().Get();
		return *this;
	}
	ByteStream& operator>> (short& val)
	{
		val = Read<short>().Get();
		return *this;
	}
	ByteStream& operator>> (char& val)
	{
		val = Read<char>().Get();
		return *this;
	}

protected:

	BS_INLINE void Enlarge(unsigned int minNewLen)
	{
		unsigned int newLen;
		if (m_PreAllocSize == 0 || m_PreAllocSize * BS_RESIZE_FACTOR < m_DataLength + minNewLen)
		{
			newLen = m_DataLength + (unsigned int)(minNewLen * BS_RESIZE_FACTOR);
		} else
		{
			newLen = (unsigned int)(m_PreAllocSize * BS_RESIZE_FACTOR);
		}

		Resize(newLen);
		
	}

private:

	unsigned char* m_ByteList;
	unsigned int m_ReadLocation;
	unsigned int m_DataLength;
	unsigned int m_PreAllocSize;
	unsigned int* m_pReferences;

	BS_INLINE bool cpy(unsigned int src, unsigned int dst, unsigned int len)
	{
		if (src + len > m_DataLength)
			return false;
		if (dst + len > m_DataLength)
			return false;
		memcpy(m_ByteList + dst, m_ByteList + src, len);
		return true;
	}


	//encipher and decipher code is taken from this page: http://www.simonshepherd.supanet.com/source.htm
	BS_INLINE void encipher(const unsigned long *const v,unsigned long *const w,
		const unsigned long * const k)
	{
		register unsigned long       y=v[0],z=v[1],sum=0,delta=0x9E3779B9,n=32;

		while(n-->0)
		{
			y += (z << 4 ^ z >> 5) + z ^ sum + k[sum&3];
			sum += delta;
			z += (y << 4 ^ y >> 5) + y ^ sum + k[sum>>11 & 3];
		}

		w[0]=y; w[1]=z;
	}

	BS_INLINE void decipher(const unsigned long *const v,unsigned long *const w,
		const unsigned long * const k)
	{
		register unsigned long       y=v[0],z=v[1],sum=0xC6EF3720,
			delta=0x9E3779B9,n=32;

		/* sum = delta<<5, in general sum = delta * n */

		while(n-->0)
		{
			z -= (y << 4 ^ y >> 5) + y ^ sum + k[sum>>11 & 3];
			sum -= delta;
			y -= (z << 4 ^ z >> 5) + z ^ sum + k[sum&3];
		}

		w[0]=y; w[1]=z;
	}

};