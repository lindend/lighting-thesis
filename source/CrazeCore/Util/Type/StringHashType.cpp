#include "StringHashType.h"
#include "../CrazeHash.h"

using namespace Craze;

//Create a map that contains all hashes to ease debugging and detecting duplicate hashes.
#ifdef _DEBUG
std::map<unsigned long, std::string>* StringHashType::m_pHashesMap = NULL;
#endif 

StringHashType::StringHashType(std::string str)
{
	SetStrId(str);
}


unsigned long StringHashType::ToHash(const std::string& str)
{
	unsigned long hash = hash32(str.c_str(), str.length());
	/*
	If we are in debug mode, check if the hash is valid.
	*/
#ifdef _DEBUG

	if (!m_pHashesMap)
	{
		m_pHashesMap = new std::map<unsigned long, std::string>();
	}
	if (m_pHashesMap->empty() || m_pHashesMap->find(hash) == m_pHashesMap->end())
	{
		(*m_pHashesMap)[hash] = str;
	}else
	{
		if ((*m_pHashesMap)[hash] != str)
		{
			LOG_CRITICAL("DUPLICATE HASHES FOUND! " + str + " yields the same hash as " + (*m_pHashesMap)[hash]);
		}
	}

	if (hash == 0)
	{
		LOG_CRITICAL("INVALID HASH FOUND! " + str + " yields the hash 0!");
	}
#endif

	return hash;
}

void StringHashType::SetStrId(std::string str)
{
	SetId(ToHash(str));
#ifdef _DEBUG
	m_DebugString = str;
#endif
}

void StringHashType::CleanUp()
{
#ifdef _DEBUG
	if (m_pHashesMap)
	{
		m_pHashesMap->clear();
		delete m_pHashesMap;
	}
#endif
}