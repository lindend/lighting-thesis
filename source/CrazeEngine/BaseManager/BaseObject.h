#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

namespace Craze
{
	/*
	The class BaseObject is not used
	*/
	class CRAZEENGINE_EXP BaseObject
	{
	public:
		/*
		GetId
		@return The objects identifier
		*/
		unsigned int GetId() { return m_ObjId; }
		/*
		SetId
		Sets the objects identifier
		@param id the identifier
		*/
		void SetId(unsigned int id) { m_ObjId = id; }
	protected:
		unsigned int m_ObjId;
	};
}