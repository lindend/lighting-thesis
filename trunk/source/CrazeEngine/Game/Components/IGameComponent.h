#pragma once
#include "EngineExport.h"

#include <string>
#include <map>

extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

namespace Craze
{
	class GameObject;

	class CRAZEENGINE_EXP ComponentParam
	{
	public:
		enum TYPE
		{
			NONE,
			FLOAT,
			INT,
			STRING
		};

		ComponentParam() : m_Type(NONE) {  }

		ComponentParam(int v) : m_Type(NONE) { SetType(INT); Value.iVal = v; }
		ComponentParam(float v) : m_Type(NONE) { SetType(FLOAT); Value.flVal = v; }
		ComponentParam(const std::string& v) : m_Type(NONE) { SetType(STRING); *Value.strVal = v; }

		ComponentParam(const ComponentParam& o) : m_Type(NONE) { *this = o; }
		ComponentParam& operator=(const ComponentParam& o)
		{
			SetType(o.m_Type);
			if (m_Type == STRING)
			{
				*Value.strVal = *o.Value.strVal;
			} else
			{
				Value = o.Value;
			}
			return *this;
		}

		~ComponentParam()
		{
			if (m_Type == STRING)
			{
				delete Value.strVal;
			}
		}

		TYPE GetType() const { return m_Type; }
		void SetType(TYPE type)
		{
			if(m_Type == type)
			{
				return;
			} else if (m_Type == STRING)
			{
				delete Value.strVal;
			} else if(type == STRING)
			{
				Value.strVal = new std::string();
			}
			m_Type = type;
		}

		void Set(float v)
		{
			SetType(FLOAT);
			Value.flVal = v;
		}
		void Set(int v)
		{
			SetType(INT);
			Value.iVal = v;
		}
		void Set(const std::string& v)
		{
			SetType(STRING);
			*Value.strVal = v;
		}

		float GetFloat() const
		{
			assert(m_Type == FLOAT);
			return Value.flVal;
		}

		int GetInt() const
		{
			assert(m_Type == INT);
			return Value.iVal;
		}

		std::string GetString() const
		{
			assert(m_Type == STRING);
			return *Value.strVal;
		}

	private:
		TYPE m_Type;
		union
		{
			float flVal;
			int iVal;
			std::string* strVal;
		} Value;
	};

	class CRAZEENGINE_EXP ComponentData
	{
	public:
		std::string m_Type;
		std::map<std::string, ComponentParam> m_Params;
	};

	class CRAZEENGINE_EXP IGameComponent
	{
	public:
		IGameComponent(GameObject* pOwner) : m_pOwner(pOwner) {}

		GameObject* GetOwner() const { return m_pOwner; }

		virtual void VUpdate(float delta) {}
		virtual std::string VGetType() const = 0;

		virtual ~IGameComponent() {}
	private:
		GameObject* m_pOwner;
	};
}