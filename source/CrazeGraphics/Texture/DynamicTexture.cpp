#include "CrazeGraphicsPCH.h"
#include "DynamicTexture.h"
#include "../Device.h"
#include "../Graphics.h"

using namespace Craze;
using namespace Craze::Graphics2;

TextureLock::TextureLock(DynamicTexture* pTexture)
{
	m_MappedResource.pData = 0;
	m_pTexture = pTexture;
}

void TextureLock::Unlock()
{
	m_pTexture->Unlock();
}
#pragma warning(push)
#pragma warning(disable:4355)
DynamicTexture::DynamicTexture(Device* pDevice) : m_Lock(this)
{
	m_pDevice = pDevice;
	m_NumBuffers = 0;
	m_ppBuffers = 0;
	m_pResourceView = 0;
	m_pTexture = 0;
}
#pragma warning(pop)

DynamicTexture::~DynamicTexture()
{
	if (m_Lock.IsLocked())
	{
		Unlock();
	}

	SAFE_RELEASE(m_pResourceView);
	SAFE_RELEASE(m_pTexture);

	if (m_ppBuffers)
	{
		for (unsigned int i = 0; i < m_NumBuffers; ++i)
		{
			SAFE_RELEASE(m_ppBuffers[i]);
		}
	}

	m_ppBuffers = 0;
}

std::shared_ptr<DynamicTexture> DynamicTexture::Create(Device* pDevice, unsigned int width, unsigned int height, TEXTURE_FORMAT format, unsigned int ringBufferSize)
{
	std::shared_ptr<DynamicTexture> pTexture(new DynamicTexture(pDevice));

	pTexture->m_ppBuffers = new ID3D11Texture2D*[ringBufferSize];
	pTexture->m_Width = width;
	pTexture->m_Height = height;
	pTexture->m_Format = (DXGI_FORMAT)format;
	pTexture->m_NumBuffers = ringBufferSize;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ArraySize = 1;
	desc.Width = width;
	desc.Height = height;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Format = (DXGI_FORMAT)format;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_STAGING;

	for (unsigned int i = 0; i < ringBufferSize; ++i)
	{
		pDevice->GetDevice()->CreateTexture2D(&desc, NULL, &pTexture->m_ppBuffers[i]);
	}

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	pDevice->GetDevice()->CreateTexture2D(&desc, NULL, (ID3D11Texture2D**)&pTexture->m_pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC rvDesc;
	rvDesc.Format = (DXGI_FORMAT)format;
	rvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rvDesc.Texture2D.MipLevels = 1;
	rvDesc.Texture2D.MostDetailedMip = 0;

	pDevice->GetDevice()->CreateShaderResourceView(pTexture->m_pTexture, &rvDesc, &pTexture->m_pResourceView);

	return pTexture;
}

TextureLock* DynamicTexture::Lock()
{
	if (m_Lock.IsLocked())
	{
		return &m_Lock;
	}

	for (unsigned int i = 0; i < m_NumBuffers; ++i)
	{
		if (SUCCEEDED(m_pDevice->GetDeviceContext()->Map(m_ppBuffers[i], 0, D3D11_MAP_WRITE, D3D11_MAP_FLAG_DO_NOT_WAIT, &m_Lock.m_MappedResource)))
		{
			m_Lock.m_pLockedResource = m_ppBuffers[i];
			return &m_Lock;
		}
	}

	//If no buffers were available, just wait until the first buffer is free
	if (SUCCEEDED(m_pDevice->GetDeviceContext()->Map(m_ppBuffers[0], 0, D3D11_MAP_WRITE, 0, &m_Lock.m_MappedResource)))
	{
		m_Lock.m_pLockedResource = m_ppBuffers[0];
		return &m_Lock;
	}

	return NULL;
}

void DynamicTexture::Unlock()
{
	if (!m_Lock.IsLocked())
	{
		return;
	}

	m_pDevice->GetDeviceContext()->Unmap(m_Lock.m_pLockedResource, 0);
	m_pDevice->GetDeviceContext()->CopyResource(m_pTexture, m_Lock.m_pLockedResource);
	m_Lock.m_MappedResource.pData = 0;
}