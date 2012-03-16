#include "CrazeGraphicsPCH.h"
#include "EffectHelper.h"

#include "D3Dcompiler.h"

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "../Device.h"
#include "../Graphics.h"

#include <functional>

using namespace Craze;
using namespace Craze::Graphics2;

#ifdef _DEBUG
#define CRAZE_DEBUG_SHADERS
#endif

class IncludeHandler : public ID3D10Include
{
public:
	IncludeHandler() : m_pData(nullptr) {}

	HRESULT __stdcall Open(D3D10_INCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID* pOutData, UINT* outDataSize)
	{
		int size;
		m_pData = gResMgr.readFileData(gFileDataLoader.addFile(pFileName), size);

		if (m_pData)
		{
			*pOutData = m_pData;
			*outDataSize = size;
			return S_OK;
		}

		return E_FAIL;
	}

	HRESULT __stdcall Close(LPCVOID)
	{
		delete [] m_pData;
		return S_OK;
	}

private:
	char* m_pData;
};

ID3D10Blob* CompileShader(const char* pData, int bufSize, const std::string& name, const std::string& mainFunction, const char* shaderProfile)
{
	unsigned int flags = D3D10_SHADER_ENABLE_STRICTNESS;

#ifdef CRAZE_DEBUG_SHADERS
	flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#else
	flags |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif

	ID3D10Blob* pResult = 0;
	ID3D10Blob* pError = 0;

	IncludeHandler incHndlr;
	
	HRESULT hr;

	if (pData)
	{
		hr = D3DCompile(pData, bufSize, NULL, NULL, &incHndlr, mainFunction.c_str(), shaderProfile, flags, 0, &pResult, &pError);
	} else
	{
		hr = D3DX11CompileFromFileA(name.c_str(), NULL, NULL, mainFunction.c_str(), shaderProfile, flags, 0, NULL, &pResult, &pError, NULL);
	}

	if (FAILED(hr))
	{
		if (pError)
		{
			std::string error = (const char*)pError->GetBufferPointer();
			LOG_ERROR("Error while compiling shader " + name + " - " + error);
			SAFE_RELEASE(pError);
		} else
		{
			LOG_ERROR("Failed to compile shader " + name + ", mysterious error! :O");
		}

		SAFE_RELEASE(pResult);
		return nullptr;

	} else if(pError)
	{
		LOG_WARNING("Warnings while compiling shader " + name + " - " + std::string((const char*)pError->GetBufferPointer()));
	}

	SAFE_RELEASE(pError);

	return pResult;
}

enum SHADERSTAGE
{
	VERTEX,
	GEOMETRY,
	PIXEL,
	COMPUTE,
	HULL,
	DOMAINSH
};

std::string GetProfile(Device* pDevice, SHADERSTAGE stage)
{
	std::string profile;

	switch (stage)
	{
	case VERTEX:
		profile = "vs";
		break;
	case GEOMETRY:
		profile = "gs";
		break;
	case PIXEL:
		profile = "ps";
		break;
	case COMPUTE:
		profile = "cs";
		break;
	case HULL:
		profile = "hs";
		break;
	case DOMAINSH:
		profile = "ds";
		break;
	}

	switch (pDevice->GetFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_11_0:
		profile += "_5_0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		profile += "_4_1";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		profile += "_4_0";
		break;
	}
	return profile;
}

ID3D11VertexShader* CreateVS(Device* pDevice, ID3D10Blob* pCode, const std::string& name, ID3D10Blob** ppByteCode)
{
	ID3D11VertexShader* pVertexShader;

	if (FAILED(pDevice->GetDevice()->CreateVertexShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &pVertexShader)))
	{
		LOG_ERROR("Error while creating vertex shader " + name + ". Unable to create vertex shader object.");
		SAFE_RELEASE(pCode);
		return nullptr;
	}

	SetDebugName(pVertexShader, name.c_str());

	if (ppByteCode)
	{
		*ppByteCode = pCode;
		pCode = 0;
	}

	SAFE_RELEASE(pCode);
		
	return pVertexShader;
}

ID3D11GeometryShader* CreateGS(Device* pDevice, ID3D10Blob* pCode, const std::string& name, ID3D10Blob** ppByteCode)
{
	ID3D11GeometryShader* pShader;

	if (FAILED(pDevice->GetDevice()->CreateGeometryShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &pShader)))
	{
		LOG_ERROR("Error while creating shader " + name + ". Unable to create shader object.");
		SAFE_RELEASE(pCode);
		return nullptr;
	}

	SetDebugName(pShader, name.c_str());

	if (ppByteCode)
	{
		*ppByteCode = pCode;
		pCode = 0;
	}

	SAFE_RELEASE(pCode);
		
	return pShader;
}

ID3D11PixelShader* CreatePS(Device* pDevice, ID3D10Blob* pCode, const std::string& name, ID3D10Blob** ppByteCode)
{
	ID3D11PixelShader* pShader;

	if (FAILED(pDevice->GetDevice()->CreatePixelShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &pShader)))
	{
		LOG_ERROR("Error while creating shader " + name + ". Unable to create shader object.");
		SAFE_RELEASE(pCode);
		return nullptr;
	}

	SetDebugName(pShader, name.c_str());

	if (ppByteCode)
	{
		*ppByteCode = pCode;
		pCode = 0;
	}

	SAFE_RELEASE(pCode);
		
	return pShader;
}

ID3D11ComputeShader* CreateCS(Device* pDevice, ID3D10Blob* pCode, const std::string& name, ID3D10Blob** ppByteCode)
{
	ID3D11ComputeShader* pShader;

	if (FAILED(pDevice->GetDevice()->CreateComputeShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &pShader)))
	{
		LOG_ERROR("Error while creating shader " + name + ". Unable to create shader object.");
		SAFE_RELEASE(pCode);
		return nullptr;
	}

	SetDebugName(pShader, name.c_str());

	if (ppByteCode)
	{
		*ppByteCode = pCode;
		pCode = 0;
	}

	SAFE_RELEASE(pCode);
		
	return pShader;
}

ID3D11HullShader* CreateHS(Device* device, ID3D10Blob* code, const std::string& name, ID3D10Blob** byteCode)
{
	ID3D11HullShader* shader;
	
	if (FAILED(device->GetDevice()->CreateHullShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &shader)))
	{
		LOG_ERROR("Error while creating shader " + name + ". Unable to create shader object.");
		SAFE_RELEASE(code);
		return nullptr;
	}

	SetDebugName(shader, name.c_str());

	if (byteCode)
	{
		*byteCode = code;
		code = nullptr;
	}
	SAFE_RELEASE(code);

	return shader;
}

ID3D11DomainShader* CreateDS(Device* device, ID3D10Blob* code, const std::string& name, ID3D10Blob** byteCode)
{
	ID3D11DomainShader* shader;
	
	if (FAILED(device->GetDevice()->CreateDomainShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &shader)))
	{
		LOG_ERROR("Error while creating shader " + name + ". Unable to create shader object.");
		SAFE_RELEASE(code);
		return nullptr;
	}

	SetDebugName(shader, name.c_str());

	if (byteCode)
	{
		*byteCode = code;
		code = nullptr;
	}
	SAFE_RELEASE(code);

	return shader;
}


//Functions that create shaders from files
template <typename ST> ST* compileShader(ST* (*createSh)(Device*, ID3D10Blob*, const std::string&, ID3D10Blob**), Device* device, const std::string& fileName, ID3D10Blob** byteCode, const std::string& mainFunction)
{
	ID3D10Blob* pCode = CompileShader(NULL, 0, fileName, mainFunction, GetProfile(device, PIXEL).c_str());

	if (!pCode)
	{
		return nullptr;
	}

	return createSh(device, pCode, fileName, byteCode);
}

ID3D11VertexShader* EffectHelper::CompileVS(Device* pDevice, const std::string& fileName, ID3D10Blob** ppByteCode, std::string mainFunction)
{	
	return compileShader(&CreateVS, pDevice, fileName, ppByteCode, mainFunction);
}

ID3D11PixelShader* EffectHelper::CompilePS(Device* pDevice, const std::string& fileName, ID3D10Blob** ppByteCode, std::string mainFunction)
{
	return compileShader(&CreatePS, pDevice, fileName, ppByteCode, mainFunction);
}

ID3D11GeometryShader* EffectHelper::CompileGS(Device* pDevice, const std::string& fileName, ID3D10Blob** ppByteCode, std::string mainFunction)
{
	return compileShader(&CreateGS, pDevice, fileName, ppByteCode, mainFunction);
}

ID3D11ComputeShader* EffectHelper::CompileCS(Device* pDevice, const std::string& fileName, ID3D10Blob** ppByteCode, std::string mainFunction)
{
	return compileShader(&CreateCS, pDevice, fileName, ppByteCode, mainFunction);
}

//Functions that create shaders from memory
template <typename ST> ST* compileShader(ST* (*createSh)(Device*, ID3D10Blob*, const std::string&, ID3D10Blob**), Device* device, SHADERSTAGE stage, const char* data, int size, const std::string& name, ID3D10Blob** byteCode, const std::string& mainFunction)
{
	ID3D10Blob* pCode = CompileShader(data, size, name, mainFunction, GetProfile(device, stage).c_str());

	if (!pCode)
	{
		return nullptr;
	}

	return createSh(device, pCode, name, byteCode);
}
ID3D11VertexShader* EffectHelper::CompileVS(Device* pDevice, const char* pData, int size, const std::string& name, ID3D10Blob** ppByteCode, std::string mainFunction)
{	
	return compileShader(&CreateVS, pDevice, VERTEX, pData, size, name, ppByteCode, mainFunction);
}

ID3D11PixelShader* EffectHelper::CompilePS(Device* pDevice, const char* pData, int size, const std::string& name, ID3D10Blob** ppByteCode, std::string mainFunction)
{
	return compileShader(&CreatePS, pDevice, PIXEL, pData, size, name, ppByteCode, mainFunction);
}

ID3D11GeometryShader* EffectHelper::CompileGS(Device* pDevice, const char* pData, int size, const std::string& name, ID3D10Blob** ppByteCode, std::string mainFunction)
{
	return compileShader(&CreateGS, pDevice, GEOMETRY, pData, size, name, ppByteCode, mainFunction);
}

ID3D11ComputeShader* EffectHelper::CompileCS(Device* pDevice, const char* pData, int size, const std::string& name, ID3D10Blob** ppByteCode, std::string mainFunction)
{
	return compileShader(&CreateCS, pDevice, COMPUTE, pData, size, name, ppByteCode, mainFunction);
}

ID3D11HullShader* EffectHelper::CompileHS(Device* device, const char* data, int size, const std::string& name, ID3D10Blob** byteCode, const std::string& mainFunction)
{
	return compileShader(&CreateHS, device, HULL, data, size, name, byteCode, mainFunction);
}

ID3D11DomainShader* EffectHelper::CompileDS(Device* device, const char* data, int size, const std::string& name, ID3D10Blob** byteCode, const std::string& mainFunction)
{
	return compileShader(&CreateDS, device, DOMAINSH, data, size, name, byteCode, mainFunction);
}

ID3D11Buffer* EffectHelper::CreateConstantBuffer(Device* pDevice, unsigned int size)
{
	if (size < 16)
	{
		size = 16;
	}

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.MiscFlags = 0;

	ID3D11Buffer* pResult = 0;

	if (FAILED(pDevice->GetDevice()->CreateBuffer(&cbDesc, NULL, &pResult)))
	{
		LOG_ERROR("Error while creating constant buffer");
		SAFE_RELEASE(pResult);
		return nullptr;
	}

	return pResult;
}
