#pragma once
#include "Texture.h"

namespace Craze
{
	namespace Graphics2
	{

		class RenderTarget : public Texture
		{
		public:
			static std::shared_ptr<RenderTarget> Create2D(Device* pDevice, unsigned int width, unsigned int height, unsigned int mipLevels, TEXTURE_FORMAT format, const char* pDebugName, bool UAV = false);
			static std::shared_ptr<RenderTarget> Create3D(Device* pDevice, unsigned int width, unsigned int height, unsigned int depth, unsigned int mipLevels, TEXTURE_FORMAT format, const char* pDebugName);

			ID3D11RenderTargetView* GetRenderTargetView();
			ID3D11UnorderedAccessView* GetUAV() { return m_pUAV; }

			RenderTarget();
			~RenderTarget();
		private:
			ID3D11RenderTargetView* m_pRenderTargetView;
			ID3D11UnorderedAccessView* m_pUAV;

			unsigned int m_Depth;
		};
	}
}