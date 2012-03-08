#include "CrazeGraphicsPCH.h"
#include "DirectionalLight.h"

#include "../Mesh.h"

#include "../Graphics.h"

using namespace Craze;
using namespace Craze::Graphics2;


DirectionalLight::DirectionalLight()
{
	m_pScreenQuad = Mesh::createScreenQuad(gpDevice);
}

void DirectionalLight::Draw()
{
	m_pScreenQuad->draw();
}

Vector3 DirectionalLight::GetDiffuse() const
{
	return m_Diffuse;
}

void DirectionalLight::SetDiffuse(const Vector3& diffuse)
{
	m_Diffuse = diffuse;
}

float DirectionalLight::GetSpecular() const
{
	return m_Specular;
}

void DirectionalLight::SetSpecular(float specular)
{
	m_Specular = specular;
}			

Vector3 DirectionalLight::GetDirection() const
{
	return m_Direction;
}

Vector3 DirectionalLight::GetUp() const
{
	return m_Up;
}

void DirectionalLight::SetDirection(const Vector3& dir, const Vector3& up)
{
	m_Direction = dir;
	m_Up = up;
}

Matrix4 DirectionalLight::GetWorld() const
{
	return Matrix4::IDENTITY;
}