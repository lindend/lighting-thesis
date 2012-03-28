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

		typedef u32* HLIGHT;

		template <typename T>
		struct LightItem
		{
			T light;
			HLIGHT handle;
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

			const PointLightArray getVisiblePointLights(const Matrix4& viewProj) const;
			const SpotLightArray getVisibleSpotLights(const Matrix4& viewProj) const;
			const DirectionalLight* getDirectionalLights(int& numLights) const;

			Camera* getCamera() { return m_camera; }

			HLIGHT addLight(const DirectionalLight& l);
			HLIGHT addLight(const SpotLight& l);
			HLIGHT addLight(const PointLight& l);
			
			PointLight* getPointLight(HLIGHT l);
			SpotLight* getSpotLight(HLIGHT l);
			DirectionalLight* getDirectionalLight(HLIGHT l);

			void removePointLight(HLIGHT light);
			void remoteSpotLight(HLIGHT light);
			void removeDirectionalLight(HLIGHT light);

			void clearLights();
			
			Vector3 AmbientLight;
			BoundingBox* m_bounds;

			std::vector<MeshItem> getModels(NODEFLAGS match = (NODEFLAGS)0) const;
		private:
			template <typename T> void freeAllLights(T lights, int numLights);

			int findFirstFreeLightSlot() const;

			boost::pool<> m_hlightPool;

			static const int MaxPointLights = 3200;
			static const int MaxSpotLights = 2000;
			static const int MaxDirectionalLights = 10;

			LightItem<PointLight> m_pointLights[MaxPointLights];
			int m_numPointLights;
			
			LightItem<SpotLight> m_spotLights[MaxSpotLights];
			int m_numSpotLights;

			LightItem<DirectionalLight> m_dirLights[MaxDirectionalLights];
			int m_numDirLights;

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
