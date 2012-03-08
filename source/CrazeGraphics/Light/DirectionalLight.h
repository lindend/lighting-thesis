#pragma once
#include "ILight.h"
#include "CrazeMath.h"

#include <memory>
#include "Memory/MemoryManager.h"

namespace Craze
{
	namespace Graphics2
	{
		class Device;
		class Mesh;

		class DirectionalLight : public ILight
		{
			CRAZE_ALLOC_ALIGN(16);
		public:
			DirectionalLight();

			virtual LIGHT_TYPE VGetType() const { return LIGHT_DIR; }

			void Draw();

			Vector3 GetDiffuse() const;
			void SetDiffuse(const Vector3& diffuse);

			float GetSpecular() const;
			void SetSpecular(float specular);
			

			Vector3 GetDirection() const;
			Vector3 GetUp() const;
			void SetDirection(const Vector3& dir, const Vector3& up);

			Matrix4 GetWorld() const;

		private:
			Matrix4 m_World;
			Matrix4 m_ViewProj;

			Vector3 m_Direction;
			Vector3 m_Up;
			Vector3 m_Diffuse;
			std::shared_ptr<Mesh> m_pScreenQuad;
			float m_Specular;

			void UpdateViewProj();
			void UpdateWorld();
		};
	}
}