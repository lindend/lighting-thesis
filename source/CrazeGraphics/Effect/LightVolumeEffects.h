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
		class TextureResource;

		class LVFirstBounceEffect : IEffect
		{
		public:
			bool initialize();
			void doFirstBounce(std::shared_ptr<RenderTarget> dummyTarget, std::shared_ptr<RenderTarget> RSMs[], std::shared_ptr<DepthStencil> RSMdepth, std::shared_ptr<UAVBuffer> outRays, const Matrix4& viewProj);

			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void destroy() { IEffect::destroy(); }
		private:
			TexturePtr m_random;
		};

		struct LightVolumeInfo
		{
			Vector3 start;
			Vector3 end;
			Vector3 cellSize;
			float numCells;
		};

		class LVInjectRaysEffect : IEffect
		{
		public:
			bool initialize();
			void injectRays(std::shared_ptr<UAVBuffer> rays, std::shared_ptr<RenderTarget> LVs[], const LightVolumeInfo& LVinfo);

			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void destroy() { IEffect::destroy(); }
		protected:
			virtual const D3D11_INPUT_ELEMENT_DESC* getLayout(int& count);
		private:
			std::shared_ptr<const TessShaderResource> m_tessShaders;
			std::shared_ptr<Buffer> m_argBuffer;
			ID3D11Buffer* m_cbuffer;
			ID3D11BlendState* m_blendState;
			ID3D11RasterizerState* m_rasterizerState;
		};

		class LVAmbientLightingEffect : IEffect
		{
		public:
			bool initialize();
			void doLighting(std::shared_ptr<RenderTarget> LVs[], std::shared_ptr<RenderTarget> gbuffers[], ID3D11ShaderResourceView* depth, const LightVolumeInfo& lvInfo);

			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void destroy() { IEffect::destroy(); }

		private:
			ID3D11Buffer* m_cbuffer;
		};
	}
}