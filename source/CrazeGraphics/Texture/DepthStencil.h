#pragma once
#include "d3d11.h"
#include "d3dx11.h"

namespace Craze
{
	namespace Graphics2
	{
		enum DEPTHSTENCIL_FORMAT
		{
			DEPTHSTENCIL_FORMAT_D24S8 = DXGI_FORMAT_R24G8_TYPELESS,
			DEPTHSTENCIL_FORMAT_D32 = DXGI_FORMAT_D32_FLOAT,
			DEPTHSTENCIL_FORMAT_D16 = DXGI_FORMAT_D16_UNORM,
			DEPTHSTENCIL_FORMAT_RAWZ = MAKEFOURCC('I', 'N', 'T', 'Z')
		};

		class Device;

		//Should this maybe be a texture?
		class DepthStencil
		{
		public:
			static std::shared_ptr<DepthStencil> Create2D(Device* pDevice, unsigned int width, unsigned int height, DEPTHSTENCIL_FORMAT format);
			static std::shared_ptr<DepthStencil> Create3D(Device* pDevice, unsigned int width, unsigned int height, unsigned int depth, DEPTHSTENCIL_FORMAT format);
			
			ID3D11DepthStencilView* GetDepthStencilView() { return m_pDepthStencilView; }
			ID3D11ShaderResourceView* GetSRV() { return m_SRV; }

			DepthStencil()
			{
				m_pDepthStencilView = 0;
				m_pDepthStencil = 0;
			}

			~DepthStencil();
		private:
			ID3D11DepthStencilView* m_pDepthStencilView;
			ID3D11Resource* m_pDepthStencil;
			ID3D11ShaderResourceView* m_SRV;

			unsigned int m_Height;
			unsigned int m_Width;
			unsigned int m_Depth;

			DEPTHSTENCIL_FORMAT m_Format;
		};
	}
}