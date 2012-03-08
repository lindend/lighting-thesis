#pragma once
#ifndef CRAZE__AGE__MAP__HPP__
#define CRAZE__AGE__MAP__HPP__

#include <map>
#include <utility>
#include "optional.hpp"

namespace Craze
{
	namespace Util
	{
		template <typename, typename> class AgeMap;

		/**
		Helper for the AgeMap class. Simply contains the age.
		*/
		template <typename Tkey, typename Tval>
		class _AgeMapEntry
		{
			friend AgeMap<Tkey, Tval>;
		protected:
			_AgeMapEntry(Tkey k, Tval v) : key(k), data(v), age(0) {}
			_AgeMapEntry() : age(0) {}
			Tkey key;
			Tval data;
			unsigned int age;

		public:
			bool operator== (const _AgeMapEntry<Tkey, Tval>& e) const
			{
				return data == e.data;
			}
			bool operator< (const _AgeMapEntry<Tkey, Tval>& e) const
			{
				return data < e.data;
			}
		};

		/**
		The AgeMap keeps track of the ages of all elements stored in the map and provides a way
		to fetch the oldest element. The elements are considered changed when they are fetched, and the
		oldest element is the element that haven't been changed for the longest.
		*/
		template <typename Tkey, typename Tval>	
		class AgeMap
		{
		public:

			AgeMap() : m_Counter(0) { }

			/**
			Returns the value associated with the key and renews the age of the element.
			@param key The key of the element to fetch.
			@return Optional object that might be containing the element, check optional.Valid() to see if the element was found.
			*/
			optional<Tval> Get(Tkey key)
			{

				_AgeMap::iterator elem = m_Map.find(key);
				if (elem == m_Map.end())
				{
					return optional<Tval>();
				}

				(*elem).second.age = m_Counter;

				return optional<Tval>((*elem).second.data);
			}

			/**
			Adds a new element to the age map.
			@param key The key of the element to add.
			@param val The value of the element to add.
			*/
			void Add(Tkey key, const Tval& val)
			{
				_Entry entry(key, val);
				entry.age = m_Counter;
				m_Map.insert(std::make_pair(key, entry));

			}

			/**
			Removes the oldest item from the age map and returns the item.
			@return An optional containing the value of the oldest element. 
			*/
			optional<Tval> RemoveOldest()
			{
				if (Size() == 0)
				{
					return optional<Tval>();
				}

				unsigned int oldest = m_Counter + 1;
				_AgeMap::iterator oldestElem;
			
				for (_AgeMap::iterator i = m_Map.begin(); i != m_Map.end(); ++i)
				{
					if ((*i).second.age < oldest)
					{
						oldestElem = i;
					}
				}

				optional<Tval> ret = optional<Tval>((*oldestElem).second.data);

				m_Map.erase(oldestElem);

				return ret;
			}

			/**
			@return An optional containing the value of the oldest element.
			*/
			optional<Tval> GetOldest()
			{
				if (Size() == 0)
				{
					return optional<Tval>();
				}

				unsigned int oldest = m_Counter + 1;
				_AgeMap::iterator oldestElem;
			
				for (_AgeMap::iterator i = m_Map.begin(); i != m_Map.end(); ++i)
				{
					if ((*i).second.age < oldest)
					{
						oldestElem = i;
					}
				}
				oldestElem->second.age = m_Counter;

				return optional<Tval>(oldestElem->second.data);
			}

			/**
			@return Number of elements in the AgeMap.
			*/
			unsigned int Size() const
			{
				return m_Map.size();
			}

			/**
			Updates the age map by increasing the timer by one step.
			*/
			void Update()
			{
				m_Counter++;
			}

			/**
			Clears all elements from the age map.
			*/
			void Clear()
			{
				m_Map.clear();
			}

		private:
			typedef _AgeMapEntry<Tkey, Tval> _Entry;
			typedef std::map<Tkey, _Entry> _AgeMap;

			_AgeMap m_Map;
			unsigned int m_Counter;
		};
	}
}


#endif /*CRAZE__AGE__MAP__HPP__*/