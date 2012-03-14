#include "CrazeGraphicsPCH.h"
#include "Texture.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"

#include "EventLogger.h"
#include "Threading/ThreadUtil.hpp"
#include "Resource/FileDataLoader.h"
#include "Resource/ResourceManager.h"

#include "../Device.h"
#include "../Graphics.h"


using namespace Craze;
using namespace Craze::Graphics2;

bool TextureResourceHandler::fileReadError(ResourceLoadData* data)
{
	return false;
}

bool TextureResourceHandler::readComplete(ResourceLoadData* data)
{
	std::shared_ptr<TextureResource> res = std::dynamic_pointer_cast<TextureResource>(data->res);
	assert(res);

	Texture* texture = Texture::CreateFromMemory(gpDevice, data->data, data->dataSize, true
#ifdef _DEBUG
				, "Texture resource"
#endif
				);

	if (texture)
	{
		texture->SetID(++m_currentID);
        res->m_texture = texture;

		return true;
	}
	return res->m_texture != nullptr;
}

unsigned int Texture::GetPixelSize(TEXTURE_FORMAT fmt)
{
	switch(fmt)
	{
	case TEXTURE_FORMAT_COLOR:
		return 4;
	case TEXTURE_FORMAT_COLOR_LINEAR:
		return 4;
	case TEXTURE_FORMAT_FLOAT:
		return 4;
	case TEXTURE_FORMAT_VECTOR2:
		return 8;
	case TEXTURE_FORMAT_VECTOR3:
		return 12;
	case TEXTURE_FORMAT_VECTOR4:
		return 16;
	case TEXTURE_FORMAT_HALF:
		return 2;
	case TEXTURE_FORMAT_HALFVECTOR2:
		return 4;
	case TEXTURE_FORMAT_HALFVECTOR4:
		return 8;
	case TEXTURE_FORMAT_8BIT_UNORM:
		return 1;
	}
	return 0;
}

Texture::Texture()
{
	m_pTexture = 0;
	m_pResourceView = 0;
	m_ID = 0;
}

Texture::~Texture()
{
	SAFE_RELEASE(m_pResourceView);
	SAFE_RELEASE(m_pTexture);

}

bool CopyIntoMips(char* pDataBuf, size_t bufLen, unsigned int numMips, D3D11_SUBRESOURCE_DATA* pInitData)
{
	size_t used = 0;

	for (unsigned int i = 0; i < numMips; ++i)
	{
		ilActiveMipmap(i == 0 ? 0 : 1);
		
		unsigned int width = ilGetInteger(IL_IMAGE_WIDTH);
		unsigned int height = ilGetInteger(IL_IMAGE_HEIGHT);

		size_t chunkSize = 4 * width * height;
		assert(used + chunkSize <= bufLen);

		void* pBuffer = pDataBuf + used;
		used += chunkSize;
		
		int err = 0;
		if (!(err = ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_UNSIGNED_BYTE, pBuffer)))
		{
			std::string errStr = WToStr(iluErrorString(err));
			LOG_ERROR("Error while copying pixels: " + errStr);
			return false;
		}

		pInitData[i].pSysMem = pBuffer;
		pInitData[i].SysMemPitch = 4 * width;
	}

	return true;
}

Texture* Craze::Graphics2::CreateTextureFromDevIL(Device* pDevice, bool gammaCorrected, const char* pDebugName)
{
	ILinfo inf;
	iluGetImageInfo(&inf);
	inf.NumMips += 1;
	
	D3D11_TEXTURE2D_DESC desc;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = gammaCorrected ? (DXGI_FORMAT)TEXTURE_FORMAT_COLOR : (DXGI_FORMAT)TEXTURE_FORMAT_COLOR_LINEAR;
	desc.Height = inf.Height;
	desc.Width = inf.Width;
	desc.MipLevels = inf.NumMips;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;

	//Allocate a buffer big enough to hold all mip levels (4 * w * h * 1.5f), mip maps increase storage by 33% but use 50% to have a slight margin
	//Can't use gMemMgr.StackAlloc since this might be called outside of the main thread
	size_t bufLen = 6 * inf.Width * inf.Height;
	char* pDataBuf = (char*)malloc(bufLen);

	D3D11_SUBRESOURCE_DATA* pInitData = (D3D11_SUBRESOURCE_DATA*)malloc(sizeof(D3D11_SUBRESOURCE_DATA) * inf.NumMips);

	Texture* pRes = nullptr;

	if (CopyIntoMips(pDataBuf, bufLen, inf.NumMips, pInitData))
	{
		ID3D11Texture2D* pTex2D = nullptr;
		if (SUCCEEDED(pDevice->GetDevice()->CreateTexture2D(&desc, pInitData, &pTex2D)))
		{
			SetDebugName(pTex2D, pDebugName);

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = desc.Format;
			srvDesc.Texture2D.MipLevels = inf.NumMips;
			srvDesc.Texture2D.MostDetailedMip = 0;

			ID3D11ShaderResourceView* pSRV = nullptr;
			if (SUCCEEDED(pDevice->GetDevice()->CreateShaderResourceView(pTex2D, &srvDesc, &pSRV)))
			{
				SetDebugName(pSRV, pDebugName);

				pRes = new Texture();
				pRes->m_pTexture = pTex2D;
				pRes->m_pResourceView = pSRV;
				pRes->m_Format = desc.Format;
				pRes->m_Dimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				pRes->m_Height = inf.Height;
				pRes->m_Width = inf.Width;
				pRes->m_NumMips = inf.NumMips;
				pRes->m_pDevice = pDevice;
				
			} else
			{
				LOG_ERROR("Unable to create shader resource view for texture");
			}			
		} else
		{
			LOG_ERROR("Unable to create D3D11 texture from DevIL data");
		}
	}

	free(pDataBuf);
	free(pInitData);

	return pRes;	
}

//Used to lock access to the DevIL library, which only supports single threaded operation
Lock gDevilLock;

Texture* Texture::CreateFromFile(Craze::Graphics2::Device* pDevice, std::string fileName, bool gammaCorrected)
{
	ScopedLock s(gDevilLock);

	ILuint img = ilGenImage();
	ilBindImage(img);

	if (!ilLoadImage(StrToW(fileName).c_str()))
	{
		ILenum err = ilGetError();
		std::string errStr = WToStr(iluErrorString(err));

		LOG_ERROR("Unable to load image: " + fileName + ". " + errStr);
		return nullptr;
	}

	iluBuildMipmaps();

	auto res = CreateTextureFromDevIL(pDevice, gammaCorrected, fileName.c_str());

	ilDeleteImage(img);

	return res;
}

Texture* Texture::CreateFromMemory(Craze::Graphics2::Device* pDevice, void* pData, unsigned long dataLength, bool gammaCorrected, const char* pDebugName)
{
	ScopedLock s(gDevilLock);

	ILuint img = ilGenImage();
	ilBindImage(img);

	if (!ilLoadL(IL_TYPE_UNKNOWN, pData, dataLength))
	{
		std::string errStr = WToStr(iluErrorString(ilGetError()));
		LOG_ERROR("Unable to load image from memory" + errStr);
		return nullptr;
	}

	iluBuildMipmaps();

	auto res = CreateTextureFromDevIL(pDevice, gammaCorrected, pDebugName ? pDebugName : "memory");

	ilDeleteImage(img);

	return res;
}

Texture* Texture::CreateFromData(Device* pDevice, unsigned int width, unsigned int height, TEXTURE_FORMAT format, void* pData, const char* pDebugName)
{
	D3D11_TEXTURE2D_DESC dsc;
	dsc.ArraySize = 1;
	dsc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dsc.CPUAccessFlags = 0;
	dsc.Format = (DXGI_FORMAT)format;
	dsc.MipLevels = 1;
	dsc.MiscFlags = 0;
	dsc.SampleDesc.Count = 1;
	dsc.SampleDesc.Quality = 0;
	dsc.Usage = D3D11_USAGE_IMMUTABLE;
	dsc.Height = height;
	dsc.Width = width;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = pData;
	data.SysMemPitch = width * GetPixelSize(format);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* pTex;

	if (FAILED(pDevice->GetDevice()->CreateTexture2D(&dsc, &data, &pTex)))
	{
		return nullptr;
	}

	SetDebugName(pTex, pDebugName);

	Texture* pResult = new Texture();

	pResult->m_pTexture = pTex;
	pResult->m_pDevice = pDevice;
	pResult->m_Width = width;
	pResult->m_Height = height;
	pResult->m_Format = (DXGI_FORMAT)format;
	pResult->m_NumMips = 0;
	pResult->m_Dimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	D3D11_SHADER_RESOURCE_VIEW_DESC rvDesc;
	ZeroMemory(&rvDesc, sizeof(rvDesc));

	rvDesc.Format = (DXGI_FORMAT)format;
	rvDesc.Texture2D.MipLevels = -1;
	rvDesc.Texture2D.MostDetailedMip = 0;
	rvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	if (FAILED(pDevice->GetDevice()->CreateShaderResourceView(pResult->m_pTexture, &rvDesc, &pResult->m_pResourceView)))
	{
		LOG_ERROR(("Could not create shader resource view when creating texture from memory"));
		delete pResult;
		return nullptr;
	}

	SetDebugName(pResult->m_pResourceView, pDebugName);

	return pResult;
}


ID3D11ShaderResourceView* Texture::GetResourceView()
{
	return m_pResourceView;
}

ID3D11Resource* Texture::GetResource()
{
	return m_pTexture;
}