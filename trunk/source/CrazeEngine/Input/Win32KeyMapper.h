#pragma once
#include "EngineExport.h"

namespace Craze
{
	enum KEYCODE;

	class CRAZEENGINE_EXP Win32KeyMapper
	{
	public:
		KEYCODE ConvertKey(unsigned int key);
	};
}