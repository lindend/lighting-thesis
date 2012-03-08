#include "CrazeGraphicsPCH.h"
#include "RenderTarget.h"
#include "../Device.h"
#include "../Graphics.h"

#include "EventLogger.h"

using namespace Craze::Graphics2;
using namespace Craze;

RenderTarget::RenderTarget() : m_pUAV(nullptr)
{
	m_pRenderTargetView = 0;
}

RenderTarget::~RenderTarget()
{
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pUAV);
}

ID3D11RenderTargetView* RenderTarget::GetRenderTargetView()
{
	return m_pRenderTargetView;
}

std::shared_ptr<RenderTarget> RenderTarget::Create2D(Device *pDevice, unsigned int width, unsigned int height, unsigned int mipLevels, TEXTURE_FORMAT format, const char* pDebugName, bool UAV)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = (DXGI_FORMAT)format;
	desc.Height = height;
	desc.MipLevels = mipLevels;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = width;

	if (UAV)
	{
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}

	std::shared_ptr<RenderTarget> pRes(new RenderTarget());
	pRes->m_Format = (DXGI_FORMAT)format;
	pRes->m_Height = height;
	pRes->m_Width = width;
	pRes->m_Dimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	pRes->m_pDevice = pDevice;
	
	if (FAILED(pDevice->GetDevice()->CreateTexture2D(&desc, NULL, (ID3D11Texture2D**)&pRes->m_pTexture)))
	{
		LOG_ERROR("Unable to create texture");
		return std::shared_ptr<RenderTarget>();
	}

	SetDebugName(pRes->m_pTexture, pDebugName);

	D3D11_SHADER_RESOURCE_VIEW_DESC resDesc;
	ZeroMemory(&resDesc, sizeof(resDesc));
	resDesc.Texture2D.MipLevels = mipLevels;
	resDesc.Format = (DXGI_FORMAT)format;
	resDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	if (FAILED(pDevice->GetDevice()->CreateShaderResourceView(pRes->m_pTexture, &resDesc, &pRes->m_pResourceView)))
	{
		LOG_ERROR("Unable to create shader resource view");
		return std::shared_ptr<RenderTarget>();
	}

	SetDebugName(pRes->m_pTexture, pDebugName);

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));
	rtDesc.Format = (DXGI_FORMAT)format;
	rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;
	
	if (FAILED(pDevice->GetDevice()->CreateRenderTargetView(pRes->m_pTexture, &rtDesc, &pRes->m_pRenderTargetView)))
	{
		LOG_ERROR("Unable to create render target view");
		return std::shared_ptr<RenderTarget>(); 
	}

	SetDebugName(pRes->m_pRenderTargetView, pDebugName);

	if (UAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = desc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		if (FAILED(pDevice->GetDevice()->CreateUnorderedAccessView(pRes->m_pTexture, &uavDesc, &pRes->m_pUAV)))
		{
			LOG_ERROR("Unable to create unordered access view for render target");
			return nullptr;
		}
		SetDebugName(pRes->m_pUAV, pDebugName);
	}

	return pRes;
}

std::shared_ptr<RenderTarget> RenderTarget::Create3D(Device *pDevice, unsigned int width, unsigned int height, unsigned int depth, unsigned int mipLevels, TEXTURE_FORMAT format, const char* pDebugName)
{
	D3D11_TEXTURE3D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = (DXGI_FORMAT)format;
	desc.Height = height;
	desc.MipLevels = mipLevels;
	desc.Depth = depth;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = width;
	desc.CPUAccessFlags = 0;

	std::shared_ptr<RenderTarget> pRes(new RenderTarget());
	pRes->m_Format = (DXGI_FORMAT)format;
	pRes->m_Height = height;
	pRes->m_Width = width;
	pRes->m_Depth = depth;
	pRes->m_Dimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	
	if (FAILED(pDevice->GetDevice()->CreateTexture3D(&desc, NULL, (ID3D11Texture3D**)&pRes->m_pTexture)))
	{
		LOG_ERROR("Unable to create texture");
		return std::shared_ptr<RenderTarget>();
	}

	SetDebugName(pRes->m_pTexture, pDebugName);

	D3D11_SHADER_RESOURCE_VIEW_DESC resDesc;
	ZeroMemory(&resDesc, sizeof(resDesc));
	resDesc.Texture3D.MipLevels = mipLevels;
	resDesc.Texture3D.MostDetailedMip = 0;
	resDesc.Format = (DXGI_FORMAT)format;
	resDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;

	if (FAILED(pDevice->GetDevice()->CreateShaderResourceView(pRes->m_pTexture, &resDesc, &pRes->m_pResourceView)))
	{
		LOG_ERROR("Unable to create shader resource view");
		return std::shared_ptr<RenderTarget>();
	}

	SetDebugName(pRes->m_pResourceView, pDebugName);

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));
	rtDesc.Format = (DXGI_FORMAT)format;
	rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
	rtDesc.Texture2DArray.ArraySize = depth;
	rtDesc.Texture2DArray.FirstArraySlice = 0;
	rtDesc.Texture2DArray.MipSlice = 0;

	if (FAILED(pDevice->GetDevice()->CreateRenderTargetView(pRes->m_pTexture, &rtDesc, &pRes->m_pRenderTargetView)))
	{
		LOG_ERROR("Unable to create render target view");
		return std::shared_ptr<RenderTarget>();
	}

	SetDebugName(pRes->m_pRenderTargetView, pDebugName);

	return pRes;
}