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
    std::shared_ptr<VertexShaderResource> res = std::dynamic_pointer_cast<VertexShaderResource>(data->res);
   	auto shader = EffectHelper::CompileVS(gpDevice, data->data, data->dataSize, data->res->name, &res->m_shaderByteCode);
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
	std::shared_ptr<GeometryShaderResource> res = std::dynamic_pointer_cast<GeometryShaderResource>(data->res);
   	auto shader = EffectHelper::CompileGS(gpDevice, data->data, data->dataSize, data->res->name);
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
	std::shared_ptr<PixelShaderResource> res = std::dynamic_pointer_cast<PixelShaderResource>(data->res);
   	auto shader = EffectHelper::CompilePS(gpDevice, data->data, data->dataSize, data->res->name);
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
	std::shared_ptr<ComputeShaderResource> res = std::dynamic_pointer_cast<ComputeShaderResource>(data->res);
   	auto shader = EffectHelper::CompileCS(gpDevice, data->data, data->dataSize, data->res->name);
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

bool TessShaderResourceHandler::readComplete(ResourceLoadData* data)
{
	std::shared_ptr<TessShaderResource> res = std::dynamic_pointer_cast<TessShaderResource>(data->res);

	auto hs = EffectHelper::CompileHS(gpDevice, data->data, data->dataSize, data->res->name);
	if (hs)
	{
		res->m_hs = hs;
	} else if(!res->m_hs)
	{
		return false;
	}

	auto ds = EffectHelper::CompileDS(gpDevice, data->data, data->dataSize, data->res->name);
	if (ds)
	{
		res->m_ds = ds;
		return true;
	} else if (res->m_ds)
	{
		return true;
	}

	return false;
}

bool HullShaderResourceHandler::readComplete(ResourceLoadData* data)
{
	std::shared_ptr<HullShaderResource> res = std::dynamic_pointer_cast<HullShaderResource>(data->res);
	auto shader = EffectHelper::CompileHS(gpDevice, data->data, data->dataSize, "HullShader");
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

bool DomainShaderResourceHandler::readComplete(ResourceLoadData* data)
{
	std::shared_ptr<DomainShaderResource> res = std::dynamic_pointer_cast<DomainShaderResource>(data->res);
	auto shader = EffectHelper::CompileDS(gpDevice, data->data, data->dataSize, "HullShader");
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