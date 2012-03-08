#pragma once
#include "ILight.h"
#include "CrazeMath.h"

#include <memory>

namespace Craze
{
	namespace Graphics2
	{
		class Device;
		class Mesh;

		class SpotLight : public ILight
		{
		private:
			Vector3 m_Position;
			Vector3 m_Direction;
			Vector3 m_Up;
			Vector3 m_Diffuse;
			float m_Specular;
			float m_FOV;
			float m_Range;
			float m_Near;
			Matrix4 m_World;
			Matrix4 m_ViewProj;

			Device* m_pDevice;

			std::shared_ptr<Mesh> m_pLightVolume;
		public:

			SpotLight(Device* pDevice);

			virtual LIGHT_TYPE VGetType() const { return LIGHT_SPOT; }

			void Draw();

			Vector3 GetDiffuse() const;
			void SetDiffuse(const Vector3& diffuse);

			float GetSpecular() const;
			void SetSpecular(float specular);
			
			Vector3 GetPosition() const;
			void SetPosition(const Vector3& v);

			Vector3 GetDirection() const;
			Vector3 GetUp() const;
			void SetDirection(const Vector3& dir, const Vector3& up);

			void SetProjection(float fov, float range, float nearPlane);
			float GetRange() const;
			float GetFOV() const;

			Matrix4 GetViewProj() const;
			Matrix4 GetWorld() const;

		private:
			void UpdateViewProj();
			void UpdateWorld();
		};
	}
}