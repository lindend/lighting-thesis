#include "CrazeGraphicsPCH.h"
#include "Camera.h"

#include "V3Math.h"


using namespace Craze::Graphics2;
using namespace Craze;

Camera::Camera()
{
	m_Up = Vector3::UP;
	m_Direction = Vector3::FORWARD;
	//Force update of view matrix
	SetPosition(Vector3::ZERO);
	SetProjection(PI / 2.f, 1.f, 100.f, 1000000.f);
}

void Camera::SetPosition(const Vector3& pos)
{
	m_Position = pos;
	m_View = Matrix4::CreateView(m_Position, m_Direction + m_Position, m_Up);
}

Vector3 Camera::GetPosition() const
{
	return m_Position;
}

void Camera::SetDirection(const Vector3& dir)
{
	m_Direction = Normalize(dir);
	m_View = Matrix4::CreateView(m_Position, m_Direction + m_Position, m_Up);
}

Vector3 Camera::GetDirection() const
{
	return m_Direction;
}

void Camera::SetUp(const Craze::Vector3 &up)
{
	m_Up = up;
	m_View = Matrix4::CreateView(m_Position, m_Direction + m_Position, m_Up);
}

Vector3 Camera::GetUp() const
{
	return m_Up;
}

void Camera::SetProjection(float fovY, float aspect, float nearPlane, float farPlane)
{
	m_Proj = Matrix4::CreatePerspectiveFov(fovY, aspect, nearPlane, farPlane);
	m_InvProj = m_Proj.GetInverse();
	m_FOV = fovY;
	m_Aspect = aspect;
	m_Far = farPlane;
	m_Near = nearPlane;
}

Matrix4 Camera::GetProjection() const
{
	return m_Proj;
}

Matrix4 Camera::GetView() const
{
	return m_View;
}

Vector3 Camera::GetPointerDirection(Vector2 scrPos)
{
	Vector4 screenSpace = Vector4(scrPos.x, scrPos.y, 0.0f, 1.0f);

	Matrix4 viewProj = m_View * m_Proj;
	Vector4 world = screenSpace * viewProj.GetInverse();
	world *= (1.f / world->w);

	return ((Vector3)world - m_Position).Normalized();
}
