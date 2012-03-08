#include "CrazeGraphicsPCH.h"
#include "TextureArray.h"
#include "../Device.h"
#include "../Graphics.h"

#include "EventLogger.h"

using namespace Craze;
using namespace Craze::Graphics2;

unsigned int FindNumMips(ID3D11Texture2D* pTex)
{
	D3D11_TEXTURE2D_DESC desc;
	pTex->GetDesc(&desc);

	return desc.MipLevels;
}

std::shared_ptr<TextureArray> TextureArray::Create(Device* pDevice, unsigned int width, unsigned int height, unsigned int arraySize, unsigned int mipLevels, TEXTURE_FORMAT format)
{
	
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ArraySize = arraySize;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Format = (DXGI_FORMAT)format;
	desc.Height = height;
	desc.MipLevels = mipLevels;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = width;

	std::shared_ptr<TextureArray> pRes(new TextureArray());
	pRes->m_Format = (DXGI_FORMAT)format;
	pRes->m_Height = height;
	pRes->m_Width = width;
	pRes->m_ArraySize = arraySize;
	pRes->m_Dimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	pRes->m_pDevice = pDevice;
	
	if (FAILED(pDevice->GetDevice()->CreateTexture2D(&desc, NULL, (ID3D11Texture2D**)&pRes->m_pTexture)))
	{
		LOG_ERROR("Unable to create texture array");
		return std::shared_ptr<TextureArray>();
	}


	pRes->m_NumMips = FindNumMips((ID3D11Texture2D*)pRes->m_pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC resDesc;
	ZeroMemory(&resDesc, sizeof(resDesc));
	resDesc.Texture2DArray.MipLevels = -1;
	resDesc.Texture2DArray.ArraySize = arraySize;
	resDesc.Texture2DArray.FirstArraySlice = 0;
	resDesc.Format = (DXGI_FORMAT)format;
	resDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

	if (FAILED(pDevice->GetDevice()->CreateShaderResourceView(pRes->m_pTexture, &resDesc, &pRes->m_pResourceView)))
	{
		LOG_ERROR("Unable to create shader resource view");
		return std::shared_ptr<TextureArray>();
	}

	return pRes;
}

ID3D11Texture2D* TextureArray::CreateMatchingTexture()
{
	ID3D11Texture2D* pRes;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Format = (DXGI_FORMAT)GetFormat();
	desc.Height = GetHeight();
	desc.MipLevels = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = GetWidth();

	m_pDevice->GetDevice()->CreateTexture2D(&desc, NULL, (ID3D11Texture2D**)&pRes);
	return pRes;
}

bool TextureArray::LoadTextureIntoSlice(unsigned int slice, std::shared_ptr<Texture> pTexture)
{
	ID3D11Resource* pTex;
	unsigned int numMips = GetNumMips();

	if (pTexture->GetFormat() != GetFormat() || pTexture->GetHeight() != GetHeight() || pTexture->GetWidth() != GetWidth())
	{
		ID3D11Texture2D* pTmpTex;
		pTmpTex = CreateMatchingTexture();

		D3DX11_TEXTURE_LOAD_INFO inf;
		D3DX11LoadTextureFromTexture(m_pDevice->GetDeviceContext(), pTexture->GetResource(), &inf, pTmpTex);

		pTex = pTmpTex;

		numMips = FindNumMips(pTmpTex);		
	} else
	{
		pTex = pTexture->GetResource();
	}

	for (unsigned int i = 0; i < numMips; ++i)
	{
		m_pDevice->GetDeviceContext()->CopySubresourceRegion(m_pTexture, D3D11CalcSubresource(i, slice, numMips), 0, 0, 0, pTex, D3D11CalcSubresource(i, 0, numMips), NULL);
	}

	return true;
}