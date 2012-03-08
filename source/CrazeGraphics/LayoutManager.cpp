#include "CrazeGraphicsPCH.h"
#include "LayoutManager.h"

#include "Device.h"
#include "Graphics.h"
#include "VertexStreams.h"
#include "Effect/IEffect.h"

using namespace Craze::Graphics2;
using namespace Craze;

void LayoutManager::Initialize()
{
	m_pInputElemDesc = VertexStreams::getStreamLayout(m_InputElemCount);
}

ID3D11InputLayout* Craze::Graphics2::LayoutManager::GetLayout(IEffect* pEffect)
{
	LayoutMap::iterator it = m_Layouts.find(pEffect);

	if (it != m_Layouts.end())
	{
		return it->second;
	}

	ID3DBlob* pByteCode = pEffect->getShaderByteCode();

	ID3D11InputLayout* pLayout = 0;
	if (FAILED(gpDevice->GetDevice()->CreateInputLayout(m_pInputElemDesc, m_InputElemCount, pByteCode->GetBufferPointer(), 
		pByteCode->GetBufferSize(), &pLayout)))
	{
		return 0;
	}

	m_Layouts[pEffect] = pLayout;

	return pLayout;
}

void LayoutManager::Destroy()
{
	for (LayoutMap::iterator i = m_Layouts.begin(); i != m_Layouts.end(); ++i)
	{
		SAFE_RELEASE(i->second);
	}
	m_Layouts.clear();
}

LayoutManager::~LayoutManager()
{
	Destroy();
}