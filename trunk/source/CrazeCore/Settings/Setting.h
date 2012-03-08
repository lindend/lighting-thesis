#pragma once
#ifndef CRAZE__SETTING__H__
#define CRAZE__SETTING__H__

#include "Util/Type/StringHashType.h"
#include <string>

#define CRAZE_DATA_BODY protected: \
	static DataType m_Type; \
	public: \
	virtual DataType VGetType() { return m_Type; }
#define CRAZE_CREATE_DATA_TYPE(classname, typeName) Craze::Settings::DataType Craze::Settings::##classname##::m_Type = Craze::StringHashType( typeName )

namespace Craze
{
	namespace Settings
	{
		typedef StringHashType DataType;

		class Idata
		{
		public:
			/*
			VGetType
			Returns the type of the data
			*/
			virtual DataType VGetType() = 0;

			virtual ~Idata() {}
		};

		template <typename T>class TData : public Idata
		{
		protected:
			T m_Data;
		public:
			T GetData() const { return m_Data; }
			void SetData(T data) { m_Data = data; }
		};

		class FloatData : public TData<float>
		{
			CRAZE_DATA_BODY
		};

		class StringData : public TData<std::string>
		{
			CRAZE_DATA_BODY	
		};

		class IntData : public TData<int>
		{
			CRAZE_DATA_BODY
		};

		class Setting
		{
		protected:
			Idata* m_pData;
		public:
			Setting(Idata* pData) { m_pData = pData; }
			Setting() { m_pData = 0; }

			void SetData(Idata* pData) { m_pData = pData; }
			Idata* GetData() { return m_pData; }

			~Setting() { if (m_pData) delete m_pData; m_pData = 0; }
		};
	}
}

#endif /*CRAZE__SETTING__H__*/