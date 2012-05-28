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

bool hasChannelInfo(uint8_t* data, int channel, int width, int height)
{

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint8_t pixelData = data[(x + y * width) * 4 + channel];
            if (pixelData > 0 && pixelData < 255)
            {
                return true;
            }
        }
    }
    return false;
}

bool copyTexture(const std::string& output, const std::string& decal, const std::string& alphaMap)
{
    ILuint decalId = loadAndConvertImage(decal);

    if (decalId == 0xFFFFFFFF)
    {
        return false;
    }

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

    if (!hasChannelInfo(data, 3, dxImg.width, dxImg.height) && alphaMap != "")
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

        int channel = 0;

        if (hasChannelInfo(alphaData, 3, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT)))
        {
            channel = 3;
        }

        for (unsigned int y = 0; y < dxImg.height; ++y)
        {
            for (unsigned int x = 0; x < dxImg.width; ++x)
            {
                unsigned int idx = x * 4 + y * dxImg.rowPitch;
                uint8_t alphaValue = 
                data[idx + 3] = alphaData[idx + channel];
            }
        }
        ilDeleteImage(alphaTex);
    }

    HRESULT hr;
    DirectX::ScratchImage flipped;
    if (FAILED(DirectX::FlipRotate(dxImg, DirectX::TEX_FR_FLIP_VERTICAL, flipped)))
    {
        char errmsg[255];
        sprintf(errmsg, "Error while flipping image: %x", hr);
        LOG_ERROR(errmsg);
        ilDeleteImage(decalId);
        return false;
    }

    ilDeleteImage(decalId);

    DirectX::ScratchImage mipChain;
    if (FAILED(hr = DirectX::GenerateMipMaps(*flipped.GetImage(0, 0, 0), DirectX::TEX_FILTER_CUBIC, 0, mipChain, false)))
    {
        char errmsg[255];
        sprintf(errmsg, "Error while building mip maps: %x", hr);
        LOG_ERROR(errmsg);
        return false;
    }

    
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


