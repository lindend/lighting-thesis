#pragma once
#include "EngineExport.h"

#include "CrazeMath.h"
#include "V3Math.h"

#include "IGameComponent.h"
#include "Memory/MemoryManager.h"

namespace Craze
{
	class CRAZEENGINE_EXP TransformListener
	{
	public:
		virtual void VUpdateTransform(const Matrix4& tfm) = 0;
	};

	class CRAZEENGINE_EXP TransformComponent : public IGameComponent
	{
		CRAZE_POOL_ALLOC_ALIGN(TransformComponent, 16);
	public:
		static TransformComponent* Create(GameObject* pOwner, lua_State* L);
		static ComponentData GetDefaultData(float x = 0.f, float y = 0.f, float z = 0.f);

		TransformComponent(GameObject* pOwner) : IGameComponent(pOwner) { SetTransform(Vector3::ZERO, Quaternion::IDENTITY); }
		TransformComponent(const Vector3& pos, const Quaternion& rot, GameObject* pOwner) : IGameComponent(pOwner) { SetTransform(pos, rot); }//, m_Position(pos), m_Rotation(rot) { UpdateTransform(); }

		//void SetPosition(const Vector3& pos) { m_Position = pos; UpdateTransform(); }
		//const Vector3& GetPosition() const { return m_Position; }

		//void SetRotation(const Quaternion& rot) { m_Rotation = rot; UpdateTransform(); }
		//const Quaternion& GetRotation() const { return m_Rotation; }

		void SetTransform(const Matrix4& tfm) { m_Transform = tfm; CallListeners(); }
		void SetTransform(const Vector3& pos, const Quaternion& rot) { m_Transform = Matrix4::CreateRotation(rot); m_Transform.SetTranslation(pos); CallListeners(); } //{ m_Position = pos; m_Rotation = rot; UpdateTransform(); }
		Matrix4 GetTransform() const { return m_Transform; }

		void AddListener(TransformListener* pListener) { m_pUpdateListeners.push_back(pListener); }
		void RemoveListener(TransformListener* pListener) { std::remove(m_pUpdateListeners.begin(), m_pUpdateListeners.end(), pListener); }

		static std::string GetType() { return "transform"; }
		virtual std::string VGetType() const { return GetType(); }

		void SetPos(const Vec3& pos) { m_Transform.m3 = _mm_setr_ps(pos.x, pos.y, pos.z, 1.f); CallListeners(); }
		Vec3 GetPos() const { return (Vector3)m_Transform.m3; }

		void SetOrientation(const Vec3& forward, const Vec3& up)
		{
			m_Transform.m1 = _mm_setr_ps(up.x, up.y, up.z, 0.f);
			Vector3 right = Cross(forward, up);
			right.w = 0.f;
			m_Transform.m2 = right.vec;
			Vector3 forw = Cross(up, right);
			forw.w = 0.f;
			m_Transform.m0 = forw.vec;
			CallListeners();
		}

	private:
		void CallListeners()
		{
			for (auto i = m_pUpdateListeners.begin(); i != m_pUpdateListeners.end(); ++i)
			{
				(*i)->VUpdateTransform(m_Transform);
			}
		}
		/*void UpdateTransform()
		{
			m_Transform = Matrix4::CreateRotation(m_Rotation);
			m_Transform.SetTranslation(m_Position);
		}*/

		Matrix4 m_Transform;
		std::vector<TransformListener*> m_pUpdateListeners;
		//Vector3 m_Position;
		//Quaternion m_Rotation;
	};
}