#pragma once

#include <string>

namespace Craze
{
	namespace SystemInfo
	{
		bool Initialize();
		unsigned int GetNumProcessors();
		const std::string& GetCurrentDir();
	};
}
