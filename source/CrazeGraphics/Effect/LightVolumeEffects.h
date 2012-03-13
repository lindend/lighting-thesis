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
			void doFirstBounce(std::shared_ptr<RenderTarget> dummyTarget, std::shared_ptr<RenderTarget> RSMs[], std::shared_ptr<DepthStencil> RSMdepth, std::shared_ptr<UAVBuffer> outRays);

			void setObjectProperties(const Matrix4& world, const Material& material) {}

			void destroy() { IEffect::destroy(); }
		private:
			const TextureResource* m_random;
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
			const TessShaderResource* m_tessShaders;
			std::shared_ptr<Buffer> m_argBuffer;
			ID3D11Buffer* m_cbuffer;
		};
	}
}