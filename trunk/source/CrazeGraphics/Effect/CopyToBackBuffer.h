#pragma once
#include <memory>

#include "IEffect.h"
#include "Texture/Texture.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;

		class CopyToBackBuffer : public IEffect
		{
		public:

			bool initialize() { return IEffect::initialize("ScreenQuad.vsh", "Simple.psh"); }

			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void doCopy(std::shared_ptr<Texture> source);

			void destroy() { IEffect::destroy(); }
		};
	}
}