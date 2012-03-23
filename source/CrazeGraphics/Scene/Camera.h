#pragma once
#include "CrazeMath.h"

#include "Memory/MemoryManager.h"

namespace Craze
{
	namespace Graphics2
	{
		class Camera
		{
			CRAZE_ALLOC_ALIGN(16);
		public:
			Camera();
			void SetProjection(float fovY, float aspect, float nearPlane, float farPlane);
			void SetOrthoProjection(float width, float height, float znear, float zfar);
			Matrix4 GetProjection() const;
			Matrix4 GetInvProjection() const { return m_InvProj; }

			void SetPosition(const Vector3& pos);
			Vector3 GetPosition() const;

			void SetDirection(const Vector3& dir);
			Vector3 GetDirection() const;

			void SetUp(const Vector3& up);
			Vector3 GetUp() const;

			Matrix4 GetView() const;
			
			float GetFovY() const
			{
				return m_FOV;
			}

			float GetAspect() const
			{
				return m_Aspect;
			}

			float GetFar() const
			{
				return m_Far;
			}

			float GetNear() const
			{
				return m_Near;
			}

			Vector3 GetPointerDirection(Vector2 scrPos);

		private:
			Matrix4 m_Proj;
			Matrix4 m_InvProj;
			Matrix4 m_View;

			Vector3 m_Position;
			Vector3 m_Direction;
			Vector3 m_Up;

			float m_FOV;
			float m_Aspect;	
			float m_Near;
			float m_Far;	
		};
	}
}