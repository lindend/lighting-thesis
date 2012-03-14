#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "Resource/ResourceManager.h"

namespace Craze
{
	class CRAZEENGINE_EXP LevelResourceHandler : public ResourceEventHandler
	{
	public:
		virtual bool PreRead(std::shared_ptr<Resource> pRes) { return true; }
		virtual bool ReadComplete(std::shared_ptr<Resource> pRes) = 0;
		virtual bool AllComplete(std::shared_ptr<Resource> pRes) = 0;

		virtual bool FileReadError(std::shared_ptr<Resource> pRes) { return false; }
	};

	class CRAZEENGINE_EXP LevelFile
	{
	public:
		void CreateNew(std::string fileName);
	};
}