#pragma once

#include <memory>

namespace Craze
{
	namespace Graphics2
	{
		class MeshData;

		void projectLightmapLowToHigh(std::shared_ptr<MeshData> lowPoly, std::shared_ptr<MeshData> highPoly);
	}
}