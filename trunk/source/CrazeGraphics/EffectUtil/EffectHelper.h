#pragma once

#include <string>

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;
		class EffectHelper
		{
		public:
            template <typename T> static const T* LoadShaderFromResource(const std::string& file)
            {
                const Resource* res = gResMgr.loadResourceBlocking(gFileDataLoader.addFile(file));
                if (res && res->getStatus() == Resource::FINISHED)
                {
                    return dynamic_cast<const T*>(res);
                }
                return nullptr;
            }

			static ID3D11VertexShader* CompileVS(Device* pDevice, const std::string& fileName, ID3D10Blob** ppByteCode = NULL, std::string mainFunction = "main");
			static ID3D11PixelShader* CompilePS(Device* pDevice, const std::string& fileName, ID3D10Blob** ppByteCode = NULL, std::string mainFunction = "main");
			static ID3D11GeometryShader* CompileGS(Device* pDevice, const std::string& fileName, ID3D10Blob** ppByteCode = NULL, std::string mainFunction = "main");
			static ID3D11ComputeShader* CompileCS(Device* pDevice, const std::string& fileName, ID3D10Blob** ppByteCode = NULL, std::string mainFunction = "main");

			static ID3D11VertexShader* CompileVS(Device* pDevice, const char* pData, int size, const std::string& name, ID3D10Blob** ppByteCode = NULL, std::string mainFunction = "main");
			static ID3D11PixelShader* CompilePS(Device* pDevice, const char* pData, int size, const std::string& name, ID3D10Blob** ppByteCode = NULL, std::string mainFunction = "main");
			static ID3D11GeometryShader* CompileGS(Device* pDevice, const char* pData, int size, const std::string& name, ID3D10Blob** ppByteCode = NULL, std::string mainFunction = "main");
			static ID3D11ComputeShader* CompileCS(Device* pDevice, const char* pData, int size, const std::string& name, ID3D10Blob** ppByteCode = NULL, std::string mainFunction = "main");

			static ID3D11Buffer* CreateConstantBuffer(Device* pDevice, unsigned int size);
		};
	}
}
