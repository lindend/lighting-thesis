#include "CrazeGraphicsPCH.h"
#include "CBufferManager.h"

#include "../Graphics.h"
#include "EffectHelper.h"
#include "CBufferHelper.hpp"
#include "Light/Light.h"
#include "V3Math.h"

using namespace Craze::Graphics2;
using namespace Craze;

CBufferManager::CBufferManager()
{
	m_pPerFrame = 0;
	m_pPerLight = 0;
	m_pPerObject;
}

CBufferManager::~CBufferManager()
{
}

void CBufferManager::Initialize(Device* pDevice)
{
	m_pDevice = pDevice;
	m_pPerFrame = EffectHelper::CreateConstantBuffer(pDevice, sizeof(Vector4) * 18);
	m_pPerLight = EffectHelper::CreateConstantBuffer(pDevice, sizeof(CBPerLight));
	m_pPerObject = EffectHelper::CreateConstantBuffer(pDevice, sizeof(Vector4) * 8);

	SetDebugName(m_pPerFrame, "Per frame data");
	SetDebugName(m_pPerLight, "Per light data");
	SetDebugName(m_pPerObject, "Per object data");

	ResetBuffers();
}

void CBufferManager::ResetBuffers()
{
    m_pDevice->GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pPerObject);
	m_pDevice->GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pPerObject);
	m_pDevice->GetDeviceContext()->GSSetConstantBuffers(0, 1, &m_pPerObject);
	m_pDevice->GetDeviceContext()->CSSetConstantBuffers(0, 1, &m_pPerObject);
	m_pDevice->GetDeviceContext()->HSSetConstantBuffers(0, 1, &m_pPerObject);
	m_pDevice->GetDeviceContext()->DSSetConstantBuffers(0, 1, &m_pPerObject);

	m_pDevice->GetDeviceContext()->VSSetConstantBuffers(2, 1, &m_pPerLight);
	m_pDevice->GetDeviceContext()->PSSetConstantBuffers(2, 1, &m_pPerLight);
	m_pDevice->GetDeviceContext()->GSSetConstantBuffers(2, 1, &m_pPerLight);
	m_pDevice->GetDeviceContext()->CSSetConstantBuffers(2, 1, &m_pPerLight);
	m_pDevice->GetDeviceContext()->HSSetConstantBuffers(2, 1, &m_pPerLight);
	m_pDevice->GetDeviceContext()->DSSetConstantBuffers(2, 1, &m_pPerLight);

	m_pDevice->GetDeviceContext()->VSSetConstantBuffers(3, 1, &m_pPerFrame);
	m_pDevice->GetDeviceContext()->PSSetConstantBuffers(3, 1, &m_pPerFrame);
	m_pDevice->GetDeviceContext()->GSSetConstantBuffers(3, 1, &m_pPerFrame);
	m_pDevice->GetDeviceContext()->CSSetConstantBuffers(3, 1, &m_pPerFrame);
	m_pDevice->GetDeviceContext()->HSSetConstantBuffers(3, 1, &m_pPerFrame);
	m_pDevice->GetDeviceContext()->DSSetConstantBuffers(3, 1, &m_pPerFrame);
}

void CBufferManager::Shutdown()
{
	SAFE_RELEASE(m_pPerObject);
	SAFE_RELEASE(m_pPerFrame);
	SAFE_RELEASE(m_pPerLight);
}

void CBufferManager::SetFrame(const CBPerFrame& data)
{
	/*
	float4 CameraPos :					packoffset(c0);
	row_major matrix ViewProj :			packoffset(c1);
	row_major matrix InvViewProj :		packoffset(c5);
	float2 InvResolution :				packoffset(c9);
	float3 AmbientLight :				packoffset(c10);
	float LPVNumCells :					packoffset(c10.w);
	float3 LPV0Start :					packoffset(c11);
	float LPV0Size :					packoffset(c11.w);
	float3 LPV1Start :					packoffset(c12);
	float LPV1Size :					packoffset(c12.w);
	float3 LPV2Start :					packoffset(c13);
	float LPV2Size :					packoffset(c13.w);
	*/

	CBufferHelper cbuffer(m_pDevice, m_pPerFrame);
	cbuffer[0] = data.cameraPos;
	cbuffer[1] = data.viewProj;
	cbuffer[5] = data.viewProj.GetInverse();
	cbuffer[9] = data.invResolution;
	cbuffer[10] = data.ambientColor;
	cbuffer[11] = data.LVStart;
	cbuffer[12] = data.LVEnd;
	cbuffer[13] = data.LVCellSize;
	cbuffer[14] = (unsigned int)data.DisableDirect;
	cbuffer[14].Y() = (unsigned int)data.DisableAO;
	cbuffer[14].Z() = (unsigned int)data.DisableIndirect;
	cbuffer[14].W() = (unsigned int)data.BoostIndirect;
	cbuffer[15] = (unsigned int)data.DisableDecal;
	cbuffer[15].Y() = (unsigned int)data.UseESM;
	cbuffer[15].Z() = data.NumObjects;

	cbuffer[16] = data.OldLVStart;
	cbuffer[17] = data.Seed;
    cbuffer[17].Y() = data.SeedY;

	cbuffer.Unmap();
}

CBPerLight CBufferManager::GetLightData(const DirectionalLight& light)
{
	CBPerLight data;
	data.lightColor = light.color;
	data.lightDir = light.dir;
	data.lightSpecular = light.specular;

	return data;
}

void CBufferManager::SetLight(const CBPerLight& data)
{
	CBufferHelper cbuffer(m_pDevice, m_pPerLight);
	*cbuffer.GetPtr<CBPerLight>() = data;
	cbuffer.Unmap();
}

void CBufferManager::SetLight(const SpotLight& light, float texelSize)
{
	CBPerLight data;
	data.lightViewProj = Matrix4::CreateView(light.pos, light.direction + light.pos, Vector3::UP) * Matrix4::CreatePerspectiveFov(light.angle * 2.f, 1.f, 3.f, light.range);
	data.lightWorld = Matrix4::CreateFromRightUpForward(Cross(light.direction, Vector3::UP), Vector3::UP, light.direction);
	data.lightAngle = light.angle;
	data.lightColor = light.color;
	data.lightDir = light.direction;
	data.lightPos = light.pos;
	data.lightRange = light.range;
	data.lightSpecular = light.specular;
	data.texelSize = texelSize;

	CBufferHelper cbuffer(m_pDevice, m_pPerLight);
	*cbuffer.GetPtr<CBPerLight>() = data;
	cbuffer.Unmap();
}

void CBufferManager::SetLight(const DirectionalLight& light, float texelSize)
{
	CBPerLight data = GetLightData(light);

	data.texelSize = texelSize;

	CBufferHelper cbuffer(m_pDevice, m_pPerLight);
	*cbuffer.GetPtr<CBPerLight>() = data;
	cbuffer.Unmap();
}

void CBufferManager::SetLight(const DirectionalLight& light, const Matrix4& viewProj, Vec3 lightPos, float texelSize)
{
	CBPerLight data = GetLightData(light);
	data.lightPos = lightPos;
	data.lightViewProj = viewProj;
	data.texelSize = texelSize;

	CBufferHelper cbuffer(m_pDevice, m_pPerLight);
	*cbuffer.GetPtr<CBPerLight>() = data;
	cbuffer.Unmap();
}

void CBufferManager::SetObject(const CBPerObject& data)
{
	CBufferHelper cbuffer(m_pDevice, m_pPerObject);
	cbuffer[0] = data.world;
	cbuffer[4] = data.normalTfm;
	cbuffer[6].W() = data.specularFactor;
	cbuffer[7] = data.lightMapIndex;
	cbuffer.Unmap();
}
