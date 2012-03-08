#include "CrazeGraphicsPCH.h"
#include "SpotLight.h"
#include "../Device.h"
#include "../Mesh.h"

using namespace Craze::Graphics2;
using namespace Craze;

SpotLight::SpotLight(Device* pDevice)
{
	m_pDevice = pDevice;
	//m_pLightVolume = Mesh::createInvertedCube(m_pDevice, 1.0f, 1.0f, 1.0f);
}

void SpotLight::Draw()
{
	m_pLightVolume->draw();
}

Vector3 SpotLight::GetDiffuse() const
{
	return m_Diffuse;
}
void SpotLight::SetDiffuse(const Vector3& diffuse)
{
	m_Diffuse = diffuse;
}

float SpotLight::GetSpecular() const
{
	return m_Specular;
}
void SpotLight::SetSpecular(float specular)
{
	m_Specular = specular;
}

Vector3 SpotLight::GetPosition() const
{
	return m_Position;
}
void SpotLight::SetPosition(const Vector3& v)
{
	m_Position = v;
	UpdateWorld();
}

Vector3 SpotLight::GetDirection() const
{
	return m_Direction;
}

Vector3 SpotLight::GetUp() const
{
	return m_Up;
}

void SpotLight::SetDirection(const Vector3& dir, const Vector3& up)
{
	m_Direction = dir;
	m_Up = up;

	UpdateViewProj();
}

void SpotLight::SetProjection(float fov, float range, float nearPlane)
{
	m_FOV = fov;
	m_Range = range;
	m_Near = nearPlane;

	UpdateViewProj();
}

float SpotLight::GetRange() const
{
	return m_Range;
}

float SpotLight::GetFOV() const
{
	return m_FOV;
}

Matrix4 SpotLight::GetViewProj() const
{
	return m_ViewProj;
}

Matrix4 SpotLight::GetWorld() const
{
	return m_World;
}

void SpotLight::UpdateWorld()
{
	float xyScale = Tan(m_FOV / 2.0f) * m_Range * 2.0f;
	m_World = Matrix4::CreateScale(xyScale, xyScale, m_Range);
	m_World.SetTranslation(Vector3(0.0f, 0.0f, m_Range / 2.0f));
	m_World = m_World * Matrix4::CreateRotation(Quaternion(Vector3::FORWARD, m_Direction));
	m_World.SetTranslation(m_Position);
	
}

void SpotLight::UpdateViewProj()
{
	UpdateWorld();
	m_ViewProj = Matrix4::CreateView(m_Position, m_Position + m_Direction, m_Up) * Matrix4::CreatePerspectiveFov(m_FOV, 1.0f, m_Near, m_Range);
}