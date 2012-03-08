#pragma once
#include "ICameraController.h"

#include "Scene/Camera.h"
#include "Event2/InputEvents.h"

namespace Craze
{
	class FreeFlyCamera : public ICameraController
	{
	public:
		FreeFlyCamera();
		FreeFlyCamera(Graphics2::Camera* pCam, float speed);
		~FreeFlyCamera();

		void BindCamera(Graphics2::Camera* pCam);
		
		void OnMouseMove(const Vector2& pos);
		void OnMouseButton(const Vector2& pos, MOUSEBUTTON button, bool pressed);
		void OnKey(KEYCODE kc, KEYSTATE ks);

		void Update(float delta);

		void SetSpeed(float speed) { m_Speed = speed; }
		float GetSpeed() const { return m_Speed; }

	private:
		void Init();

		Graphics2::Camera* m_pCamera;

		float m_Speed;

		Vector2 m_MousePos;

		bool m_MouseDown : 1;
		bool m_MoveForward : 1;
		bool m_MoveBack : 1;
		bool m_MoveLeft : 1;
		bool m_MoveRight : 1;

	};
}