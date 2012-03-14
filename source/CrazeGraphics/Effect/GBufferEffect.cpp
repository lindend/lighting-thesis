#include "CrazeGraphicsPCH.h"
#include "GBufferEffect.h"
#include "../Graphics.h"
#include "../EffectUtil/CBufferHelper.hpp"
#include "../Scene/Camera.h"

using namespace Craze::Graphics2;
using namespace Craze;

GBufferEffect::GBufferEffect()
{
	m_vertexShaderTerrain = 0;
}

GBufferEffect::~GBufferEffect()
{
	destroy();
}

void GBufferEffect::destroy()
{
	IEffect::destroy();
	SAFE_RELEASE(m_vertexShaderTerrain);
}

bool GBufferEffect::initialize()
{
	//if (m_pVertexShaderTerrain = EffectHelper::CompileVS(gpDevice, "Shaders/Terrain.vsh"))
	{
		return IEffect::initialize("DefaultMesh.vsh", "GBuffers.psh");
	}

	destroy();
	return false;
}

ID3D11ShaderResourceView* optSRV(TexturePtr tex)
{
	if (tex.get())
	{
		return tex->GetResourceView();
	}
	return nullptr;
}

void GBufferEffect::setObjectProperties(const Matrix4& world, const Material& material)
{
	Matrix4 worldView = world * m_camera->GetView();
	CBPerObject data;
	data.specularFactor = material.m_specular;
	data.world = worldView * m_camera->GetProjection();
	data.normalTfm = Transpose(Inverse(Matrix3(world)));
	data.lightMapIndex = material.m_lightMapIndex;

	ID3D11ShaderResourceView* const srvs[4] = { material.m_decal.get() ? material.m_decal->GetResourceView() : nullptr, nullptr, optSRV(material.m_lightMap), optSRV(material.m_skyVisMap) };
	gpDevice->GetDeviceContext()->PSSetShaderResources(0, 4, srvs);

	if (material.m_type == MT_NORMAL)
	{
		gpDevice->SetShader(m_vs->m_shader);
	} else if(material.m_type == MT_TERRAIN)
	{
		gpDevice->SetShader(m_vertexShaderTerrain);

		//ID3D11ShaderResourceView* pSRV = material.m_pSpecialTextures[0]->GetResourceView();
		//gpDevice->GetDeviceContext()->VSSetShaderResources(7, 1, &pSRV);
	}

	gpDevice->GetCbuffers()->SetObject(data);

}
