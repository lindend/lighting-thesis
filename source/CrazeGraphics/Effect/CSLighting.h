#pragma once

#include "D3D11.h"
#include "../EffectUtil/ShaderResource.h"

namespace Craze
{
	namespace Graphics2
	{
		class Camera;
		class SRVBuffer;
		struct LightArray;

		class CSLighting
		{
		public:
			bool initialize();
			void destroy();

			void run(const Camera* pCamera, ID3D11ShaderResourceView* pGBufSRVs[4], ID3D11UnorderedAccessView* pTarget, const LightArray& lights);
		private:
			std::shared_ptr<SRVBuffer> m_lightBuffer;
			std::shared_ptr<const ComputeShaderResource> m_cs;
			ID3D11Buffer* m_cb;
			static const unsigned int MaxLights = 3000;
		};
	}
}
