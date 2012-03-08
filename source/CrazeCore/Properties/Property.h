#pragma once
#include "Util/Type/StringHashType.h"

#define CRAZE_DATA_BODY public: \
	static DataType Type; \
	virtual DataType VGetType() { return Type; }
#define CRAZE_CREATE_DATA_TYPE(classname, typeName) Craze::Properties::DataType Craze::Properties::##classname##::Type = Craze::StringHashType( typeName )

namespace Craze
{
	namespace Properties
	{
		typedef StringHashType DataType;

		class IData
		{
		public:
			/*
			VGetType
			Returns the type of the data
			*/
			virtual DataType VGetType() = 0;

			virtual ~IData() {}
		};

		template <typename T>class TData : public IData
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

		class Property
		{
		protected:
			IData* m_pData;
		public:
			Property(IData* pData) { m_pData = pData; }
			Property() { m_pData = 0; }

			void SetData(IData* pData) { m_pData = pData; }
			IData* GetData() { return m_pData; }

			~Property() { if (m_pData) delete m_pData; m_pData = 0; }
		};
	}
}