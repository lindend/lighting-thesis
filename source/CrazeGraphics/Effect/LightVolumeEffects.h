#pragma once
#include <memory>

#include "IEffect.h"

namespace Craze
{
	namespace Graphics2
	{
		class Texture;
		class RenderTarget;
		class DepthStencil;
		class UAVBuffer;
		class TextureResource;

		class InjectRaysEffect : IEffect
		{
		public:
			bool initialize();
			void inject(std::shared_ptr<RenderTarget> dummyTarget, std::shared_ptr<RenderTarget> RSMs[], std::shared_ptr<DepthStencil> RSMdepth, std::shared_ptr<UAVBuffer> outRays);

			void setObjectProperties(const Matrix4& world, const Material& material) {}

		private:
			const TextureResource* m_random;
		};
	}
}