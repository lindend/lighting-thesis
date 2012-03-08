#pragma once

namespace Craze
{
	namespace Graphics2
	{
		class Camera;
	}

	class ICameraController
	{
	public:
		virtual void BindCamera(Graphics2::Camera* pCamera) = 0;
		virtual void Update(float delta) = 0;
	};
}