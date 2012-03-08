#pragma once
#include "EngineExport.h"

#include <vector>

#include "Memory/MemoryManager.h"

namespace Craze
{
	class IGameComponent;

	typedef unsigned __int32 ObjectId;
	class CRAZEENGINE_EXP GameObject
	{
		CRAZE_POOL_ALLOC(GameObject);
	public:
		GameObject() : m_Ref(0) {}
		~GameObject();
		void Update(float delta);

		void AddComponent(IGameComponent* pComponent);

		template <class T> T* GetComponent() const
		{
			std::string type = T::GetType();
			for (auto i = m_pComponents.begin(); i != m_pComponents.end(); ++i)
			{
				if ((*i)->VGetType() == type)
				{
					return (T*)(*i);
				}
			}
			return nullptr;
		}

		void SetId(ObjectId id) { m_Id = id; }
		ObjectId GetId() const { return m_Id; }

		void SetScriptRef(int ref) { m_Ref = ref; }
		int GetScriptRef() const { return m_Ref; }
	private:
		GameObject(const GameObject&);
		GameObject& operator=(const GameObject&);

		std::vector<IGameComponent*> m_pComponents;
		ObjectId m_Id;
		int m_Ref;
	};
}