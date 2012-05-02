#pragma once
#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class DrawRaysEffect : public IEffect
		{
		public:
            DrawRaysEffect();

			bool initialize() { return IEffect::initialize("RayTracing/DrawRay.vsh", "RayTracing/DrawRay.psh", "RayTracing/DrawRay.gsh"); }
			void set() { IEffect::set(); }
			void destroy() { IEffect::destroy(); }
			void reset() { IEffect::reset(); }

			void setObjectProperties(const Matrix4& tfm, const Material& mat) {}
		};
	}
}