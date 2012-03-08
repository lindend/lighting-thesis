#pragma once

#ifndef CRAZEGFX
#define CRAZEGFX
#endif

#include "../Shaders/Light.incl"
#include "Vector3.h"

namespace Craze
{
	namespace Graphics2
	{
		Light CreatePointLight(const Vector3& pos, float range, const Vector3& color);
		Light createDirectionalLight(const Vector3& dir, const Vector3& color);

		struct LightArray
		{
			SoAV3* pPositions;
			Vector4* pColors;
			float* pRanges;
			int numLights;
			int numPosSoA;
		};
	}
}