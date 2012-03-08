#pragma once

namespace Craze
{
	class Vector3;

	namespace Random
	{
		float GetFloat(float min = 0.f, float max = 1.f);
		Vector3 GetVector3(float min = 0.f, float max = 1.f);

	}
}