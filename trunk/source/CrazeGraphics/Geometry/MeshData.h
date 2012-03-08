#pragma once
#include <fstream>

#include "../Material.h"

#include "../CrazeMath/CrazeMath.h"

#include "Util/Type/StringHashType.h"

namespace Craze
{
	namespace Graphics2
	{
		struct Vertex;
		struct LightMapVertex;
		struct SkinnedVertex;

		struct MaterialData
		{
			MATERIAL_TYPE type;
			std::string decalFileName;
			std::string bumpFileName;

			u64 decalId;
			u64 bumpId;

			float specularFactor;
		};

		std::shared_ptr<class MeshData> LoadMeshFromMemory(const char* pData, unsigned long length);
		bool SaveMeshToFile(std::shared_ptr<MeshData> pMesh, const std::string& fileName);

		
		class MeshData
		{
			CRAZE_ALLOC();
		public:

            MeshData() : m_pIndices(0), m_pVertices(0), m_pLightMapped(0), m_pSkinned(0), m_NumIndices(0), m_NumVertices(0)
            {
            }

			void SetIndices(const unsigned short* pIndices, int numIndices) __restrict;
			const unsigned short* GetIndices() const { return m_pIndices; }
			int GetNumIndices() const { return m_NumIndices; }

			void SetVertices(const Vertex* pVertices, int numVertices);
			void SetVertices(const LightMapVertex* pVertices, int numVertices);
			void SetVertices(const SkinnedVertex* pVertices, int numVertices);

			const Vertex* GetPosNormalUv() const { return m_pVertices; }
			const LightMapVertex* GetLightMapUv() const { return m_pLightMapped; }
			const SkinnedVertex* GetSkinned() const { return m_pSkinned; }

			int GetNumVertices() const { return m_NumVertices; }

			MaterialData m_Material;

			void SetName(const std::string& name) { m_Name = name; }
			std::string GetName() const { return m_Name; }

			//Read and write to/from a craze mesh file loaded into memory
			bool ParseFile(const char* pData, unsigned long dataLength);
			void WriteToFile(FILE* f);

			u32 getFileSize();

			void writeToFile(std::ofstream& f);

			virtual ~MeshData();

        private:
			std::string m_Name;
            Vertex* m_pVertices;
			LightMapVertex* m_pLightMapped;
			SkinnedVertex* m_pSkinned;
            unsigned short* m_pIndices;

            int m_NumVertices;
            int m_NumIndices;

			MeshData(const MeshData&);
			MeshData& operator=(const MeshData&);
		};
	}
}
