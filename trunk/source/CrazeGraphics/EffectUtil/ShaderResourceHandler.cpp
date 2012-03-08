#include "CrazeGraphicsPCH.h"
#include "ShaderResourceHandler.h"

#include "../Graphics.h"
#include "../Device.h"
#include "EffectHelper.h"
#include "ShaderResource.h"

using namespace Craze;
using namespace Craze::Graphics2;

bool VertexShaderResourceHandler::readComplete(ResourceLoadData* data)
{
    VertexShaderResource* res = dynamic_cast<VertexShaderResource*>(data->res);
   	auto shader = EffectHelper::CompileVS(gpDevice, data->data, data->dataSize, "VertexShader", &res->m_shaderByteCode);
	if (shader)
	{
		res->m_shader = shader;
		return true;
	} else if(res->m_shader)
	{
		return true;
	}
    
    return false;
}

bool GeometryShaderResourceHandler::readComplete(ResourceLoadData* data)
{
   GeometryShaderResource* res = dynamic_cast<GeometryShaderResource*>(data->res);
   	auto shader = EffectHelper::CompileGS(gpDevice, data->data, data->dataSize, "GeometryShader");
	if (shader)
	{
		res->m_shader = shader;
		return true;
	} else if(res->m_shader)
	{
		return true;
	}
    
    return false;
}

bool PixelShaderResourceHandler::readComplete(ResourceLoadData* data)
{
	PixelShaderResource* res = dynamic_cast<PixelShaderResource*>(data->res);
   	auto shader = EffectHelper::CompilePS(gpDevice, data->data, data->dataSize, "PixelShader");
	if (shader)
	{
		res->m_shader = shader;
		return true;
	} else if(res->m_shader)
	{
		return true;
	}
    
    return false;
}

bool ComputeShaderResourceHandler::readComplete(ResourceLoadData* data)
{
	ComputeShaderResource* res = dynamic_cast<ComputeShaderResource*>(data->res);
   	auto shader = EffectHelper::CompileCS(gpDevice, data->data, data->dataSize, "ComputeShader");
	if (shader)
	{
		res->m_shader = shader;
		return true;
	} else if(res->m_shader)
	{
		return true;
	}
    
    return false;
}
