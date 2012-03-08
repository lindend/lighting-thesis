#include "CrazeGraphicsPCH.h"
#include "IEffect.h"

#include "Resource/ResourceManager.h"

#include "../Graphics.h"
#include "../Device.h"
#include "../VertexStreams.h"

using namespace Craze;
using namespace Craze::Graphics2;

void IEffect::destroy()
{
	if (m_vs)
	{
		m_vs->release();
		m_vs = nullptr;
	}
	if (m_gs)
	{
		m_gs->release();
		m_gs = nullptr;
	}
	if (m_ps)
	{
		m_ps->release();
		m_ps = nullptr;
	}
	if (m_inputLayout)
	{
		m_inputLayout->Release();
		m_inputLayout = nullptr;
	}
}

void IEffect::set()
{
	gpDevice->SetCurrentEffect(this);
	if (m_vs)
	{
        gpDevice->SetShader(m_vs->m_shader);
	}
	if (m_gs)
	{
        gpDevice->SetShader(m_gs->m_shader);
	}

	if (m_ps)
	{
        gpDevice->SetShader(m_ps->m_shader);
	}
}

void IEffect::reset()
{
	gpDevice->SetShader((ID3D11GeometryShader*)0);
}

const D3D11_INPUT_ELEMENT_DESC* IEffect::getLayout(int& count)
{
	return VertexStreams::getStreamLayout(count);
}

bool IEffect::initialize(const char* vsFile, const char* psFile, const char* gsFile)
{
	if (vsFile)
	{
	    m_vs = EffectHelper::LoadShaderFromResource<VertexShaderResource>(vsFile);
	    if (m_vs)
	    {
	        m_byteCode = m_vs->m_shaderByteCode;

			int count;
			auto desc = getLayout(count);
			if (FAILED(gpDevice->GetDevice()->CreateInputLayout(desc, count, m_byteCode->GetBufferPointer(), m_byteCode->GetBufferSize(), &m_inputLayout)))
			{
				return false;
			}
	    } else
	    {
	        return false;
	    }
	}

	if (gsFile)
	{
	    if (!(m_gs = EffectHelper::LoadShaderFromResource<GeometryShaderResource>(gsFile)))
	    {
	        return false;
	    }
	}

	if (psFile)
	{
	    if (!(m_ps = EffectHelper::LoadShaderFromResource<PixelShaderResource>(psFile)))
	    {
	        return false;
	    }
	}

	return true;
}
