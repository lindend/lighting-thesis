#include "CrazeGraphicsPCH.h"
#include "ShadingEffect.h"

#include "../Graphics.h"
#include "../EffectUtil/CBufferHelper.hpp"

using namespace Craze;
using namespace Craze::Graphics2;

ShadingEffect::ShadingEffect()
{
	m_psDefault = 0;
	m_psTerrain = 0;
	m_vsDefault = 0;
	m_vsTerrain = 0;
}

ShadingEffect::~ShadingEffect()
{
	destroy();
}

void ShadingEffect::destroy()
{
	SAFE_RELEASE(m_vsDefault);
	SAFE_RELEASE(m_vsTerrain);
	SAFE_RELEASE(m_psDefault);
	SAFE_RELEASE(m_psTerrain);
}

bool ShadingEffect::initialize()
{
	
	if ((m_vsDefault = EffectHelper::CompileVS(gpDevice, "Shaders/DefaultMesh.vsh", &m_byteCode)))// && (m_pVertexShaderTerrain = EffectHelper::CompileVS(gpDevice, "Shaders/Terrain.vsh", &pVSByteCode)))
	{
		if ((m_psDefault = EffectHelper::CompilePS(gpDevice, "Shaders/DefaultMeshShading.psh")))// && (m_pPixelShaderTerrain = EffectHelper::CompilePS(gpDevice, "Shaders/TerrainShading.psh")))
		{
			return true;
		}
	}

	return false;
}

void ShadingEffect::set(Texture* lightAccumulation)
{
	gpDevice->SetCurrentEffect(this);

	ID3D11ShaderResourceView* sr = lightAccumulation->GetResourceView();
	gpDevice->GetDeviceContext()->PSSetShaderResources(4, 1, &sr);

	gpDevice->SetShader(m_vsDefault);
	gpDevice->SetShader(m_psDefault);

}

void ShadingEffect::setObjectProperties(const Matrix4& world, const Material& material)
{
	CBPerObject data;
	data.world = world;

	gpDevice->GetCbuffers()->SetObject(data);

	if (material.m_decal)
	{
		ID3D11ShaderResourceView* sr = material.m_decal->GetResourceView();
		gpDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &sr);
	} else
	{
		ID3D11ShaderResourceView* sr = nullptr;
		gpDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &sr);
	}

	if (material.m_type == MT_NORMAL)
	{

		gpDevice->SetShader(m_vsDefault);
		gpDevice->SetShader(m_psDefault);	

	} else if(material.m_type == MT_TERRAIN)
	{

		gpDevice->SetShader(m_vsTerrain);
		gpDevice->SetShader(m_psTerrain);

		ID3D11ShaderResourceView* srv[2]; 
			
		//pSRV[0] = material.m_pSpecialTextures[0]->GetResourceView();
		//pSRV[1] = material.m_pSpecialTextures[1]->GetResourceView();
		gpDevice->GetDeviceContext()->VSSetShaderResources(7, 2, srv);
	}
}

void ShadingEffect::reset()
{
	ID3D11ShaderResourceView* nullView = 0;
	gpDevice->GetDeviceContext()->PSSetShaderResources(4, 1, &nullView);
}