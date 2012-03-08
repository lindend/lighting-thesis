#pragma once
#ifndef STRING__HASH__TYPE__H__
#define STRING__HASH__TYPE__H__

#include "Type.h"
#include <string>

#ifdef _DEBUG
#include <map>
#include "EventLogger.h"
#endif

namespace Craze
{

	/**
	String hash based type, hashes a string with a custom Craze-hashing function
	optimized for speed! Since I know nothing of hashing functions it probably
	sucks though :] In debug mode the hash type keeps track of all hashes
	calculated and reports if two different strings yield the same hash.
	*/
	class StringHashType : public Type
	{
	public:
		/**
		Initializes the string hash type to the specified string.
		@param str The string to be hashed.
		*/
		StringHashType(std::string str);
		/**
		Copy constructor
		*/
		StringHashType(const StringHashType& sht)
		{
			m_Id = sht.m_Id;
#ifdef _DEBUG
			m_DebugString = sht.m_DebugString;
#endif
		}

		/**
		Static function that hashes the string to an unsigned long.
		Uses SSE2 intrinsics for lightning fast 128-bit at the time
		hashing! Benchmarked to 11.5 GB / s on my C2D 2.8 GHz. Data
		preferably n * 16 bytes large for best performance.
		@param str The string to hash to an unsigned long.
		@return The hash.
		*/
		static unsigned long ToHash(const std::string& str);
		/**
		Sets the Id of the type by a string that is hashed.
		@param idStr The new type identifier to be hashed.
		*/
		void SetStrId(std::string idStr);

		bool operator==(const StringHashType& sht) const
		{
			return m_Id == sht.m_Id;
		}

		bool operator== (const std::string& str) const
		{
			return m_Id == ToHash(str);
		}

		const StringHashType& operator=(const StringHashType& sht)
		{
			m_Id = sht.m_Id;
#ifdef _DEBUG
			m_DebugString = sht.m_DebugString;
#endif
			return sht;
		}

		const StringHashType& operator=(const std::string& str)
		{
			m_Id = ToHash(str);
#ifdef _DEBUG
			m_DebugString = str;
#endif
			return *this;
		}

		bool operator< (const StringHashType& sht) const
		{
			return m_Id < sht.m_Id;
		}
		
		/**
		Frees up some memory used when debugging the
		hash types.
		*/
		static void CleanUp();

		/**
		If the application is built in debug mode the stringhashtype
		will contain a debug string to help identification of types,
		if not the function will return an empty string.
		@return The debug identifier string.
		*/
		std::string GetDebugString() const
		{ 
#ifdef _DEBUG
			return m_DebugString;
#else
			return "";
#endif
		}
	private:
#ifdef _DEBUG
		static std::map<unsigned long, std::string>* m_pHashesMap;
		std::string m_DebugString;
#endif /*_DEBUG*/
	};

}
#endif /*STRING__HASH__TYPE__H__*/