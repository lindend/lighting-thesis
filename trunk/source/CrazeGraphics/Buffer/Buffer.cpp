#include "CrazeGraphicsPCH.h"
#include "Buffer.h"

#include "../Device.h"
#include "Graphics.h"

using namespace Craze;
using namespace Craze::Graphics2;

Buffer::~Buffer()
{
	SAFE_RELEASE(m_pBuffer);
}

ID3D11Buffer* createBuffer(Device* pDevice, unsigned long size, unsigned long stride, UINT miscFlags, UINT bindFlags, D3D11_USAGE usage, const void* pData, const char* debugName)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.StructureByteStride = stride;
	desc.ByteWidth = size;
	desc.MiscFlags = miscFlags;
	desc.BindFlags = bindFlags;
	desc.Usage = usage;
	desc.CPUAccessFlags = usage == D3D11_USAGE_DEFAULT ? 0 : D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = pData;
	
	ID3D11Buffer* pBuffer = nullptr;

	if (SUCCEEDED(pDevice->GetDevice()->CreateBuffer(&desc, pData ? &initData : nullptr, &pBuffer)))
	{
		SetDebugName(pBuffer, debugName);
		return pBuffer;
	}

	return nullptr;
}

ID3D11ShaderResourceView* createSRV(Device *pDevice, ID3D11Resource* pBuffer, unsigned long numElems, const char* debugName)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.NumElements = numElems;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;

	ID3D11ShaderResourceView* pSRV;
	if (SUCCEEDED(pDevice->GetDevice()->CreateShaderResourceView(pBuffer, &srvDesc, &pSRV)))
	{
		SetDebugName(pSRV, debugName);
		return pSRV;
	}

	return nullptr;
}

std::shared_ptr<Buffer> Buffer::CreateArg(Device *pDevice, int size, const void *pData, const char *pDebugName)
{
	ID3D11Buffer* pBuffer;
	if (pBuffer = createBuffer(pDevice, size, 0, D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS, 0, D3D11_USAGE_DEFAULT, pData, pDebugName))
	{
		return std::shared_ptr<Buffer>(CrNew Buffer(pDevice, pBuffer));
	}
	return nullptr;
}


std::shared_ptr<GeometryBuffer> GeometryBuffer::Create(Device* pDevice, TYPE t, const void *pData, unsigned int stride, unsigned int numElems, bool dynamic, const char *pDebugName)
{
	ID3D11Buffer* pBuf = createBuffer(pDevice, stride * numElems, 0, 0, 
							t == VERTEX ? D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER, 
							dynamic ? D3D11_USAGE_DYNAMIC: D3D11_USAGE_DEFAULT , pData, pDebugName);

	if (pBuf)
	{
		std::shared_ptr<GeometryBuffer> pResult(CrNew GeometryBuffer(pDevice, pBuf));
		pResult->m_FirstElem = 0;
		pResult->m_LastElem = numElems;
		pResult->m_NumElems = numElems;
		pResult->m_Stride = stride;
		pResult->m_Type = t;
		return pResult;
	}
	LOG_ERROR("Unable to create geomtry buffer");
	return nullptr;
}

std::shared_ptr<SRVBuffer> SRVBuffer::CreateStructured(Device *pDevice, int elemSize, int numElems, const void *pData, bool dynamic, const char *pDebugName)
{
	ID3D11Buffer* pBuf = createBuffer(pDevice, elemSize * numElems, elemSize, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, D3D11_BIND_SHADER_RESOURCE, 
								dynamic ? D3D11_USAGE_DYNAMIC: D3D11_USAGE_DEFAULT , pData, pDebugName);
	if (pBuf)
	{
		ID3D11ShaderResourceView *pSRV = createSRV(pDevice, pBuf, numElems, pDebugName);
		if (pSRV)
		{
			return std::shared_ptr<SRVBuffer> (CrNew SRVBuffer(pDevice, pBuf, pSRV));
		}
		pBuf->Release();
	}
	LOG_ERROR("Unable to create structured SRVBuffer");
	return nullptr;
}

std::shared_ptr<SRVBuffer> SRVBuffer::CreateRaw(Device *pDevice, DXGI_FORMAT format, int size, const void *pData, const char *pDebugName)
{
	ID3D11Buffer *pBuf = createBuffer(pDevice, size, 0, 0, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, pData, pDebugName);
	
	if (pBuf)
	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(pBuf, format, 0, 1);
		ID3D11ShaderResourceView* pSRV;
		if (SUCCEEDED(gpDevice->GetDevice()->CreateShaderResourceView(pBuf, &srvDesc, &pSRV)))
		{
			SetDebugName(pSRV, pDebugName);
			return std::shared_ptr<SRVBuffer>(CrNew SRVBuffer(pDevice, pBuf, pSRV));
		}
		pBuf->Release();
	}
	return nullptr;
}

std::shared_ptr<SRVBuffer> SRVBuffer::CreateRawArg(Device *pDevice, int size, const void *pData, const char *pDebugName)
{
	ID3D11Buffer *pBuf = createBuffer(pDevice, size, 0, D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, pData, pDebugName);
	
	if (pBuf)
	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(pBuf, DXGI_FORMAT_R32G32B32A32_UINT, 0, 1);
		ID3D11ShaderResourceView* pSRV;
		if (SUCCEEDED(gpDevice->GetDevice()->CreateShaderResourceView(pBuf, &srvDesc, &pSRV)))
		{
			SetDebugName(pSRV, pDebugName);
			return std::shared_ptr<SRVBuffer>(CrNew SRVBuffer(pDevice, pBuf, pSRV));
		}
		pBuf->Release();
	}
	return nullptr;
}

std::shared_ptr<UAVBuffer> UAVBuffer::Create(Device *pDevice, int elemSize, int numElems, bool appendConsume, const char *pDebugName)
{
	void *pData = malloc(elemSize * numElems);
	ZeroMemory(pData, elemSize * numElems);

	ID3D11Buffer *pBuffer = 0;
	if (pBuffer = createBuffer(pDevice, elemSize * numElems, elemSize, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 
								D3D11_USAGE_DEFAULT, pData, pDebugName))
	{
		free(pData);
		pData = 0;

		ID3D11ShaderResourceView *pSRV = 0;
		if (pSRV = createSRV(pDevice, pBuffer, numElems, pDebugName))
		{
			ID3D11UnorderedAccessView * pUAV = 0;
			CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pBuffer, DXGI_FORMAT_UNKNOWN, 0, numElems, appendConsume ? D3D11_BUFFER_UAV_FLAG_APPEND : 0);
			if (SUCCEEDED(pDevice->GetDevice()->CreateUnorderedAccessView(pBuffer, &uavDesc, &pUAV)))
			{
				return std::shared_ptr<UAVBuffer>(CrNew UAVBuffer(pDevice, pBuffer, pSRV, pUAV));
			}
		}
	}

	free(pData);
	pData = 0;
	return nullptr;
}

bool GeometryBuffer::WriteData(const void *pData, int numElems)
{
	if (!m_IsDynamic)
	{
		LOG_ERROR("Attempt to write data to a static geometry buffer");
		return false;
	}

	if (numElems > m_NumElems)
	{
		LOG_ERROR("Attempt to write more data to a dynamic buffer than it can possibly hold");
		return false;
	}

	int newFirst;
	D3D11_MAPPED_SUBRESOURCE mapped;
	if (numElems > (m_NumElems - m_LastElem))
	{
		if (FAILED(m_pDevice->GetDeviceContext()->Map(GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			LOG_ERROR("Unable to map geomtry buffer with DISCARD flag");
			return false;
		}
		newFirst = 0;
	} else
	{
		if (FAILED(m_pDevice->GetDeviceContext()->Map(GetBuffer(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped)))
		{
			LOG_ERROR("Unable to map geometry buffer with NO_OVERWRITE flag");
			return false;
		}
		newFirst = m_LastElem;
	}

	memcpy(((char *)mapped.pData) + newFirst * m_Stride, pData, numElems * m_Stride);
	m_pDevice->GetDeviceContext()->Unmap(GetBuffer(), 0);
	
	m_FirstElem = newFirst;
	m_LastElem = newFirst + numElems;
	return true;
}