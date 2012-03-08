#pragma once
#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;

		class AmbientLighting : public IEffect
		{
		public:

			bool initialize() { return IEffect::initialize("ScreenQuad.vsh", "AmbientLighting.psh"); }

			void set() { IEffect::set(); }
			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void reset() { IEffect::reset(); }

			void destroy() { IEffect::destroy(); }
		};
	}
}