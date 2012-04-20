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
		PointLight CreatePointLight(const Vector3& pos, float range, const Vector3& color);
		DirectionalLight createDirectionalLight(const Vector3& dir, const Vector3& color);
		SpotLight createSpotLight(const Vector3& pos, const Vector3& dir, float angle, float range, const Vector3& color);

		struct PointLightArray
		{
			PointLight* pointLights;
			int numLights;
		};

		struct SpotLightArray
		{
			SpotLight* spotLights;
			int numLights;
		};
	}
}