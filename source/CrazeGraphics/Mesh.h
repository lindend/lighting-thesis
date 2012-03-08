#pragma once
#include <memory>

#include "Memory/MemoryManager.h"
#include "Resource/ResourceManager.h"
#include "Intersection/BoundingBox.h"

#include "VertexStreams.h"

namespace Craze
{

	namespace Graphics2
	{
		class Device;
		class MeshData;
		struct Vertex;
		struct LightMapVertex;
		struct SkinnedVertex;
		class GeometryBuffer;

		extern Device* gpDevice;

		struct MaterialData;

		class Mesh : public Resource
		{
			friend class MeshResourceHandler;

			CRAZE_POOL_ALLOC(Mesh);
		public:
			Mesh(Device* device);

			void setIndexData(const unsigned short* indices, unsigned int numIndices);
			void setVertexData(const Vertex* vertices, unsigned int numVertices);
			void setVertexData(const LightMapVertex* vertices, unsigned int numVertices);
			void setVertexData(const SkinnedVertex* vertices, unsigned int numVertices);

			void draw() const;

			static std::shared_ptr<Mesh> createFromData(Device* device, MeshData* data);
			static std::shared_ptr<Mesh> createScreenQuad(Device* device);

			bool createFromMemory(const char* data, unsigned long length);

			~Mesh() {}

            const BoundingBox& getBoundingBox() const { return *m_boundingBox; }
            std::shared_ptr<MeshData> getMeshData() const { return m_meshData; }

		private:

			std::shared_ptr<GeometryBuffer> m_indices;
			VertexStreams m_vertexStream;

			BoundingBox* m_boundingBox;

			std::shared_ptr<MeshData> m_meshData;

			Device* m_device;

		//Resource stuff:
		protected:
			virtual void onDestroy()
			{
			    delete m_boundingBox;
			}
        private:
            virtual Resource* createInstance()
            {
                return CrNew Mesh(gpDevice);
            }
		};

		class MeshResourceHandler : public ResourceEventHandler
		{
		public:
			MeshResourceHandler() { m_readCompleteMT = true; m_allCompleteMT = true;}

			virtual bool preRead(Resource* ) { return true; }
			virtual bool readComplete(ResourceLoadData* loadData);
			virtual bool allComplete(ResourceLoadData*) { return true; }

			virtual bool fileReadError(ResourceLoadData*) { return false; }

		};

	}
}
