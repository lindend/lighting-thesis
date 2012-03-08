#pragma once
#include <vector>
#include <memory>

#include "Resource/ResourceManager.h"

#include "Mesh.h"
#include "Material.h"

namespace Craze
{
	namespace Graphics2
	{
	    struct MeshItem
		{
            const Mesh* mesh;
            Material material;
		};

		class Model : public Resource
		{
            CRAZE_POOL_ALLOC(Model);
        public:
            virtual void onDestroy()
            {
            }

            const std::vector<MeshItem>& getMeshes() const { return m_meshes; }

            bool createFromMemory(const char* data, unsigned int length);
        private:
			std::vector<MeshItem> m_meshes;
		};

		bool saveModel(std::string fileName, std::vector<std::shared_ptr<MeshData>> meshes);

		class ModelResourceHandler : public ResourceEventHandler
		{
		public:
			ModelResourceHandler() { m_readCompleteMT = true; m_allCompleteMT = true;}

			virtual bool preRead(Resource* ) { return true; }
			virtual bool readComplete(ResourceLoadData* loadData);
			virtual bool allComplete(ResourceLoadData*) { return true; }

			virtual bool fileReadError(ResourceLoadData*) { return false; }

			virtual Resource* createResource(u32, u64) { return CrNew Model(); }
		};
	}
}
