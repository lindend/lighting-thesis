#pragma once
#include "EngineExport.h"

#include "IGameComponent.h"
#include "CoreComponents.h"
#include "CrazeGraphics/Scene/ModelNode.h"
#include "Resource/ResourceManager.h"
#include "../Game.h"

#include "Model.h"

namespace Craze
{
	class CRAZEENGINE_EXP TriMeshComponent : public IGameComponent, public TransformListener
	{
		CRAZE_POOL_ALLOC(TriMeshComponent);
	public:
		TriMeshComponent(TransformComponent* pTfmComp, const Resource* pRes, Level* pLevel, GameObject* pOwner);

		static TriMeshComponent* Create(Level* pLevel, GameObject* pOwner, lua_State* L);

		~TriMeshComponent();

		static std::string GetType() { return "mesh"; }
		virtual std::string VGetType() const { return GetType(); }

		virtual void VUpdate(float delta);
		virtual void VUpdateTransform(const Matrix4& tfm) { if (m_model) { m_model->setTransform(tfm); } }
	
	private:
		TransformComponent* m_pTfmComp;
		const Graphics2::Model* m_modelRes;
		Graphics2::ModelNode* m_model;
		Level* m_pLevel;

		TriMeshComponent(const TriMeshComponent&);
		TriMeshComponent operator=(const TriMeshComponent&);
	};
}