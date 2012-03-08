#pragma once

namespace Craze
{
	namespace Graphics2
	{
		enum LIGHT_TYPE
		{
			LIGHT_SPOT = 0,
			LIGHT_DIR = 1,
		};

		class ILight
		{
			virtual LIGHT_TYPE VGetType() const = 0;
		};
	}
}