#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "Resource/ResourceManager.h"

namespace Craze
{
	class CRAZEENGINE_EXP LevelResourceHandler : public ResourceEventHandler
	{
	public:
		virtual bool PreRead(Resource* pRes) { return true; }
		virtual bool ReadComplete(Resource* pRes) = 0;
		virtual bool AllComplete(Resource* pRes) = 0;

		virtual bool FileReadError(Resource* pRes) { return false; }
	};

	class CRAZEENGINE_EXP LevelFile
	{
	public:
		void CreateNew(std::string fileName);
	};
}