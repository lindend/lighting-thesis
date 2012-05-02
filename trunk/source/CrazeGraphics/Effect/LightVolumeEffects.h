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
		class Buffer;
		class SRVBuffer;
		class TextureResource;
		class Camera;
        struct LightVolumeInfo;

		class LVAmbientLightingEffect : IEffect
		{
		public:
			bool initialize();
			void doLighting(std::shared_ptr<RenderTarget> LVs[], std::shared_ptr<RenderTarget> gbuffers[], ID3D11ShaderResourceView* depth, const LightVolumeInfo& lvInfo);

			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void destroy() { IEffect::destroy(); }

		};
	}
}