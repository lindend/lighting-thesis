#pragma once
#include <memory>

#include "IEffect.h"
#include "Light/Light.h"
#include "Texture/RenderTarget.h"

#include "Matrix4.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;

		class LightingEffect : public IEffect
		{
		public:

			bool initialize() { return IEffect::initialize("ScreenQuad.vsh", "DirLight.psh"); }

			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void doLighting(const DirectionalLight& l, const Matrix4& lightViewProj, std::shared_ptr<RenderTarget> shadowMap);

			void destroy() { IEffect::destroy(); }
		};
	}
}