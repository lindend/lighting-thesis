#pragma once
#include "IEffect.h"
#include "../Graphics.h"
#include "../Device.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;

		class NullEffect : public IEffect
		{
		public:

			bool initialize() { return IEffect::initialize("ScreenQuad.vsh", "UI.psh"); }

			void set() { IEffect::set(); }
			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void reset() { IEffect::reset(); }

			void destroy() { IEffect::destroy(); }
		};
	}
}