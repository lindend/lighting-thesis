#include <string>

#include "IL/il.h"
#include "IL/ilu.h"
#include "DirectXTex/DirectXTex.h"

#include "EventLogger.h"
#include "StrUtil.hpp"

ILuint loadAndConvertImage(const std::string& img)
{
    ILuint texId = ilGenImage();
	ilBindImage(texId);

	if (!ilLoadImage(StrToW(img).c_str()))
	{
		ILenum errnum = ilGetError();
		std::string error = WToStr(iluErrorString(ilGetError()));
		LOG_ERROR(error);
		ilDeleteImage(texId);
		return -1;
	}
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    return texId;
}

bool copyTexture(const std::string& output, const std::string& decal, const std::string& alphaMap)
{
    ILuint decalId = loadAndConvertImage(decal);

    uint8_t* data = (uint8_t*)ilGetData();
    
    DirectX::Image dxImg;
    dxImg.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    dxImg.height = ilGetInteger(IL_IMAGE_HEIGHT);
    dxImg.width = ilGetInteger(IL_IMAGE_WIDTH);
    dxImg.rowPitch = dxImg.width * 4;
    dxImg.slicePitch = dxImg.rowPitch * dxImg.height;
    dxImg.pixels = data;

    DirectX::TexMetadata metadata;
    metadata.arraySize = 1;
    metadata.depth = 1;
    metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
    metadata.format = dxImg.format;
    metadata.height = dxImg.height;
    metadata.width = dxImg.width;
    metadata.mipLevels = 1;
    metadata.miscFlags = 0;

    if (alphaMap != "")
    {
        ILuint alphaTex = loadAndConvertImage(alphaMap);

        if (ilGetInteger(IL_IMAGE_WIDTH) != dxImg.width ||
            ilGetInteger(IL_IMAGE_HEIGHT) != dxImg.height)
        {
            ilDeleteImage(decalId);
            ilDeleteImage(alphaTex);
            return false;
        }

        uint8_t* alphaData = (uint8_t*)ilGetData();

        for (unsigned int y = 0; y < dxImg.height; ++y)
        {
            for (unsigned int x = 0; x < dxImg.width; ++x)
            {
                unsigned int idx = x * 4 + y * dxImg.rowPitch;
                data[idx + 3] = alphaData[idx];
            }
        }

    }

    HRESULT hr;
    DirectX::ScratchImage flipped;
    if (FAILED(DirectX::FlipRotate(dxImg, DirectX::TEX_FR_FLIP_VERTICAL, flipped)))
    {
        char errmsg[255];
        sprintf(errmsg, "Error while flipping image: %x", hr);
        LOG_ERROR(errmsg);
        return false;
    }

    DirectX::ScratchImage mipChain;
    if (FAILED(hr = DirectX::GenerateMipMaps(*flipped.GetImage(0, 0, 0), DirectX::TEX_FILTER_CUBIC, 0, mipChain, false)))
    {
        char errmsg[255];
        sprintf(errmsg, "Error while building mip maps: %x", hr);
        LOG_ERROR(errmsg);
        return false;
    }

    ilDeleteImage(decalId);
    
    DirectX::ScratchImage compressed;
    if (FAILED(hr = DirectX::Compress(mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), DXGI_FORMAT_BC3_UNORM_SRGB, DirectX::TEX_COMPRESS_DEFAULT, 0.5f, compressed)))
    {
        char errmsg[255];
        sprintf(errmsg, "Error while building mip maps: %x", hr);
        LOG_ERROR(errmsg);
        return false;
    }

    if (FAILED(hr = DirectX::SaveToDDSFile(compressed.GetImages(), compressed.GetImageCount(), compressed.GetMetadata(), DirectX::DDS_FLAGS_NONE, StrToW(output + ".dds").c_str())))
    {
        char errmsg[255];
        sprintf(errmsg, "Error while building mip maps: %x", hr);
        LOG_ERROR(errmsg);
        return false;
    }
    return true;
}


