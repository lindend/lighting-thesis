#include "CrazeGraphicsPCH.h"
#include "CSLighting.h"

#include "../EffectUtil/EffectHelper.h"
#include "../Graphics.h"
#include "../Device.h"
#include "../Scene/Camera.h"
#include "../Light/Light.h"
#include "../Buffer/Buffer.h"

#include "../EffectUtil/CBufferHelper.hpp"

#include "Resource/ResourceManager.h"

using namespace Craze;
using namespace Craze::Graphics2;

bool CSLighting::initialize()
{
	m_cb = EffectHelper::CreateConstantBuffer(gpDevice, 6 * sizeof(Vector4));
	if (!m_cb)
	{
		return false;
	}
	m_lightBuffer = SRVBuffer::CreateStructured(gpDevice, sizeof(PointLight), MaxLights, nullptr, true, "Light buffer");

    m_cs = EffectHelper::LoadShaderFromResource<ComputeShaderResource>("Lighting.csh");
    return (m_cs != 0);
}

void CSLighting::destroy()
{
	SAFE_RELEASE(m_cb);
	m_cs = nullptr;
	m_lightBuffer = nullptr;
}

void CSLighting::run(const Camera* camera, ID3D11ShaderResourceView* gbufSrvs[4], ID3D11UnorderedAccessView* target, const PointLightArray& lights)
{
	PROFILEF();
	if (lights.numLights > MaxLights)
	{
		LOG_WARNING("The number of lights is larger than the current light count maximum! Some lights will be ignored");
	}

	{
		PROFILE("CSLighting::Run - Copy light data");

		D3D11_MAPPED_SUBRESOURCE mapped;
		gpDevice->GetDeviceContext()->Map(m_lightBuffer->GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

		PointLight* data = (PointLight*)mapped.pData;
		memcpy(data, lights.pointLights, sizeof(PointLight) * lights.numLights);
	}

	gpDevice->GetDeviceContext()->Unmap(m_lightBuffer->GetBuffer(), 0);

	Vector2 vp = gpDevice->GetViewPort();
	gpDevice->GetDeviceContext()->CSSetShader(m_cs->m_shader, 0, 0);

	{
		const float n = camera->GetNear();
		const float f = camera->GetFar();
		const float projA = f / (f - n);
		const float projB = (-f * n) / (f - n);

		CBufferHelper cb(gpDevice, m_cb);
		cb[0] = camera->GetInvProjection();
		cb[4] = lights.numLights;
		cb[4].Y() = Vector2(1.f / vp.x, 1.f / vp.y);
		cb[5] = Vector2(projA, projB);
	}

	ID3D11ShaderResourceView* srvs[5] = {0};
	std::copy(gbufSrvs, &gbufSrvs[4], srvs);
	srvs[4] = *m_lightBuffer;

	gpDevice->GetDeviceContext()->CSSetShaderResources(0, 5, srvs);
	gpDevice->GetDeviceContext()->CSSetConstantBuffers(0, 1, &m_cb);
	UINT count = -1;
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &target, &count);

	unsigned int threadsX = (unsigned int)vp.x / 16;
	unsigned int threadsY = (unsigned int)vp.y / 16;
	gpDevice->GetDeviceContext()->Dispatch(threadsX, threadsY, 1);

	ID3D11ShaderResourceView* pNullSRVs[5] = {0};
	gpDevice->GetDeviceContext()->CSSetShaderResources(0, 5, pNullSRVs);
	target = nullptr;
	gpDevice->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &target, &count);
}
