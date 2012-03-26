#pragma once
#include <memory>

#include "boost/pool/object_pool.hpp"

#include "Camera.h"
#include "ModelNode.h"
#include "../Light/Light.h"
#include "../Material.h"

namespace Craze
{
	class Frustum;
	class BoundingBox;

	namespace Graphics2
	{
		class Device;
		class TerrainChunk;
		class Mesh;
		class Material;
		class DrawList;
		struct MeshItem;

		class Model;

		enum NODEFLAGS
		{
			NODE_DYNAMIC = 0x1,
			NODE_DISPLAYABLE = 0x2,
			NODE_ALLFLAGS = NODE_DYNAMIC | NODE_DISPLAYABLE,
		};

		class Scene
		{
			CRAZE_ALLOC_ALIGN(16);
		public:
			Scene(Device* device);
			~Scene();

			ModelNode* addModel(std::shared_ptr<const Model> model, NODEFLAGS flags = NODE_ALLFLAGS);
			void removeModel(ModelNode* model);

			//MeshInstance* addMesh(std::shared_ptr<Mesh> pMesh, NODEFLAGS flags);
			//MeshInstance* addTerrain(TerrainChunk* terrain, NODEFLAGS flags);

			void update();

			void build() {}
			void buildDrawList(DrawList* drawList, const Matrix4& viewProj) const;

			LightArray getVisibleLights(const Matrix4& viewProj);

			Camera* getCamera() { return m_camera; }

			Light* addLight(const Light& l);
			void addSun(const Light& l) { m_sunLight = l; }
			void removeLight(Light* light);
			const Light& getSun() const { return m_sunLight; }

			Vector3 AmbientLight;
			BoundingBox* m_bounds;

			std::vector<MeshItem> getModels(NODEFLAGS match = (NODEFLAGS)0) const;
		private:
			int findFirstFreeLightSlot() const;

			static const int MaxLightsInScene = 3200;
			Light m_lights[MaxLightsInScene];
			int m_numLights;
			int m_activeLights[MaxLightsInScene / 32];
			int m_maxLightIndex;

			Light m_sunLight;
			bool m_hasSun;

			static const int MaxMeshesInScene = 3200;

			std::vector<ModelNode*> m_models;
			//A list of the bounding boxes of each mesh in the scene
			//Perform culling by traversing this list
			BoundingBox* m_meshBBs;
			int m_currentBB;
			std::vector<MeshItem> m_meshes;

			Device* m_device;
			Camera* m_camera;
		};
	}
}
