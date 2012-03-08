#pragma once
#include "CrazeEngine.h"

#define MEMFN1(FunctionName) std::bind((&FunctionName), this, std::placeholders::_1)
#define MEMFN2(FunctionName) std::bind((&FunctionName), this, std::placeholders::_1, std::placeholders::_2)
#define MEMFN3(FunctionName) std::bind((&FunctionName), this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define MEMFN4(FunctionName) std::bind((&FunctionName), this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
#define MEMFN5(FunctionName) std::bind((&FunctionName), this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)

namespace Craze
{

	template<class T, int id = 0> class GlobalEvent
	{
	public:
		typedef std::function<T> Listener;

		static void AddListener(const void* pListener, Listener callback)
		{
			s_Listeners.insert(std::make_pair(pListener, callback));
		}

		static void RemoveListener(const void* pListener)
		{
			s_Listeners.erase(pListener);
		}

		static void Clear()
		{
			s_Listeners.clear();
		}

	public:
		static std::map<const void*, Listener> s_Listeners;
	};

	template <class T> class ListenerHelper
	{
	public:
		ListenerHelper(void* pListener, typename T::Listener listener)
		{
			m_pListener = pListener;
			T::AddListener(m_pListener, listener);
		}

		~ListenerHelper()
		{
			T::RemoveListener(m_pListener);
		}

	private:
		void* m_pListener;
	};

	template <class T, int id> std::map<const void*, typename GlobalEvent<T, id>::Listener> GlobalEvent<T, id>::s_Listeners;// = std::map<void*, typename Craze::GlobalEvent<T>::Listener>();

	template<class A1, int id = 0> class GlobalEvent1 : public GlobalEvent<void(A1), id>
	{
	public:
		static void Notify(A1 _a1)
		{
			for (auto i = s_Listeners.begin(); i != s_Listeners.end(); ++i)
			{
				i->second(_a1);
			}
		}
	};


	template<class A1, class A2, int id = 0> class GlobalEvent2 : public GlobalEvent<void(A1, A2), id>
	{
	public:
		static void Notify(A1 _a1, A2 _a2)
		{
			for (auto i = s_Listeners.begin(); i != s_Listeners.end(); ++i)
			{
				i->second(_a1, _a2);
			}
		}
	};

	template<class A1, class A2, class A3, int id = 0> class GlobalEvent3 : public GlobalEvent<void(A1, A2, A3), id>
	{
	public:
		static void Notify(A1 _a1, A2 _a2, A3 _a3)
		{
			for (auto i = s_Listeners.begin(); i != s_Listeners.end(); ++i)
			{
				i->second(_a1, _a2, _a3);
			}
		}
	};

	template<class A1, class A2, class A3, class A4, int id = 0> class GlobalEvent4 : public GlobalEvent<void(A1, A2, A3, A4), id>
	{
	public:
		static void Notify(A1 _a1, A2 _a2, A3 _a3, A4 _a4)
		{
			for (auto i = s_Listeners.begin(); i != s_Listeners.end(); ++i)
			{
				i->second(_a1, _a2, _a3, _a4);
			}
		}
	};

	template<class A1, class A2, class A3, class A4, class A5, int id = 0> class GlobalEvent5 : public GlobalEvent<void(A1, A2, A3, A4, A5), id>
	{
	public:
		static void Notify(A1 _a1, A2 _a2, A3 _a3, A5 _a5)
		{
			for (auto i = s_Listeners.begin(); i != s_Listeners.end(); ++i)
			{
				i->second(_a1, _a2, _a3, _a4, _a5);
			}
		}
	};
}