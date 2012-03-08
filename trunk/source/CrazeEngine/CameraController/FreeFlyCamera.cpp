#include "CrazeEngine.h"
#include "FreeFlyCamera.h"
#include "Application/Application.h"

using namespace Craze;

FreeFlyCamera::FreeFlyCamera() : m_pCamera(nullptr), m_Speed(200.f)
{
	Init();
}

FreeFlyCamera::FreeFlyCamera(Graphics2::Camera* pCamera, float speed) : m_pCamera(pCamera), m_Speed(speed)
{
	Init();
}

void FreeFlyCamera::Init()
{
	MouseButton::AddListener(this, MEMFN3(FreeFlyCamera::OnMouseButton));
	Keyboard::AddListener(this, MEMFN2(FreeFlyCamera::OnKey));

	m_MouseDown = false;
	m_MoveForward = false;
	m_MoveBack = false;
	m_MoveLeft = false;
	m_MoveRight = false;
}

FreeFlyCamera::~FreeFlyCamera()
{
	MouseButton::RemoveListener(this);
	//MouseMove::RemoveListener(this);
	Keyboard::RemoveListener(this);
}

void FreeFlyCamera::BindCamera(Graphics2::Camera* pCam)
{
	m_pCamera = pCam;
}

void FreeFlyCamera::OnMouseMove(const Vector2& pos)
{
	if (m_MouseDown)
	{

	}
}

void FreeFlyCamera::OnMouseButton(const Vector2& pos, MOUSEBUTTON button, bool pressed)
{
	if (button == BTN_RIGHT)
	{
		m_MousePos = gInput.GetMousePosition();
		m_MouseDown = pressed;
	}
}

void FreeFlyCamera::OnKey(KEYCODE kc, KEYSTATE ks)
{
	if (ks == KS_TYPE)
		return;

	if (kc == KC_W)
	{
		m_MoveForward = ks == KS_PRESS;
	} else if(kc == KC_S)
	{
		m_MoveBack = ks == KS_PRESS;
	} else if(kc == KC_D)
	{
		m_MoveRight = ks == KS_PRESS;
	} else if(kc == KC_A)
	{
		m_MoveLeft = ks == KS_PRESS;
	}
}

void FreeFlyCamera::Update(float delta)
{
	if (m_pCamera)
	{
		Vector3 forwDir = m_pCamera->GetDirection();
		Vector3 sideDir = forwDir.Cross(m_pCamera->GetUp());

		Vector3 move = forwDir * m_Speed * m_MoveForward;
		move += forwDir * -m_Speed * m_MoveBack;

		move += sideDir * -m_Speed * m_MoveRight;
		move += sideDir * m_Speed * m_MoveLeft;

		move *= delta;

		m_pCamera->SetPosition(move + m_pCamera->GetPosition());

		if (m_MouseDown)
		{
			Vector2 pos = gInput.GetMousePosition();
			Vector2 delta = pos - m_MousePos;
			gInput.SetMousePosition(m_MousePos);

			Vector3 p = m_pCamera->GetDirection();
			Vector3 right = p.Cross(m_pCamera->GetUp());
			Quaternion rot = Quaternion(right, delta.y / 300.0f).Normalized();
			p = rot.Transform(p);
			rot = Quaternion(Vector3::UP, -delta.x / 300.0f).Normalized();
			p = rot.Transform(p);
			m_pCamera->SetDirection(Vector3::Normalize(p));
		}
	}
}