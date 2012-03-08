#include "CrazeGraphicsPCH.h"
#include "DepthStencil.h"
#include "../Device.h"
#include "../Graphics.h"

#include "EventLogger.h"

using namespace Craze::Graphics2;
using namespace Craze;

DepthStencil::~DepthStencil()
{
	SAFE_RELEASE(m_SRV);
	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pDepthStencil);
}

std::shared_ptr<DepthStencil> DepthStencil::Create2D(Craze::Graphics2::Device *pDevice, unsigned int width, unsigned int height, Craze::Graphics2::DEPTHSTENCIL_FORMAT format)
{
	
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.Format = (DXGI_FORMAT)format;
	depthStencilDesc.Height = height;
	depthStencilDesc.Width = width;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;

	std::shared_ptr<DepthStencil> pRes(new DepthStencil());
	pRes->m_Width = width;
	pRes->m_Height = height;
	pRes->m_Depth = 0;
	pRes->m_Format = format;

	if (FAILED(pDevice->GetDevice()->CreateTexture2D(&depthStencilDesc, NULL, (ID3D11Texture2D**)&pRes->m_pDepthStencil)))
	{
		LOG_ERROR("Could not create depth stencil buffer");
		pRes.reset((DepthStencil*)nullptr);
		return pRes;
	}

	if (FAILED(pDevice->GetDevice()->CreateDepthStencilView(pRes->m_pDepthStencil, NULL, &pRes->m_pDepthStencilView)))
	{
		LOG_ERROR("Could not create depth stencil render target view");
		pRes.reset((DepthStencil*)nullptr);
	}

	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC((ID3D11Texture2D*)pRes->m_pDepthStencil, D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT, 0, 1, 0, 0);
	if (FAILED(pDevice->GetDevice()->CreateShaderResourceView(pRes->m_pDepthStencil, &srvDesc, &pRes->m_SRV)))
	{
		LOG_ERROR("Could not create depth stencil shader resource view");
		pRes.reset((DepthStencil*)nullptr);
	}

	return pRes;
}

std::shared_ptr<DepthStencil> DepthStencil::Create3D(Craze::Graphics2::Device *pDevice, unsigned int width, unsigned int height, unsigned int depth, Craze::Graphics2::DEPTHSTENCIL_FORMAT format)
{
	
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.ArraySize = depth;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.Format = (DXGI_FORMAT)format;
	depthStencilDesc.Height = height;
	depthStencilDesc.Width = width;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;

	std::shared_ptr<DepthStencil> pRes(new DepthStencil());
	pRes->m_Width = width;
	pRes->m_Height = height;
	pRes->m_Depth = depth;
	pRes->m_Format = format;

	if (FAILED(pDevice->GetDevice()->CreateTexture2D(&depthStencilDesc, NULL, (ID3D11Texture2D**)&pRes->m_pDepthStencil)))
	{
		LOG_ERROR("Could not create depth stencil buffer");
		pRes.reset((DepthStencil*)nullptr);
		return pRes;
	}

	if (FAILED(pDevice->GetDevice()->CreateDepthStencilView(pRes->m_pDepthStencil, NULL, &pRes->m_pDepthStencilView)))
	{
		LOG_ERROR("Could not create depth stencil render target view");
		pRes.reset((DepthStencil*)nullptr);
	}

	return pRes;
}