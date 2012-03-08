#include "CrazeModel.h"

#include "assimp\assimp.hpp"
#include "assimp\aiPostProcess.h"
#include "assimp\aiScene.h"
#include "assimp\aiPostProcess.h"

#include "VertexStreams.h"

using namespace Craze;
using namespace Craze::Graphics2;

#include "StrUtil.hpp"


Vec3 aiV3ToCraze(aiVector3D vec)
{
	Vec3 v = { vec.x, vec.y, vec.z };
	return v;
}

Vector2 aiV3ToCrazeV2(aiVector3D vec)
{
	return Vector2(vec.x, vec.y);
}

std::vector<std::shared_ptr<Craze::Graphics2::MeshData>> loadModelAssimp(const std::string& fileName)
{
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(fileName, aiProcess_ConvertToLeftHanded | aiProcess_FindDegenerates | aiProcess_FindInvalidData | aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices
		| aiProcess_LimitBoneWeights | aiProcess_SortByPType | aiProcess_Triangulate);

	std::vector<std::shared_ptr<Craze::Graphics2::MeshData>> res;

	if (!pScene)
	{
		LOG_ERROR("Unable to load model file " + fileName);
		return res;
	}

	if (!pScene->HasMeshes())
	{
		LOG_WARNING("No meshes found in file: " + fileName);
		return res;
	}

	if (!pScene->HasAnimations())
	{
		res.reserve(pScene->mNumMeshes);

		for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
		{
			const aiMesh* pMesh = pScene->mMeshes[i];
			if (pMesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE && pMesh->HasTextureCoords(0) && pMesh->mMaterialIndex >= 0)
			{
				Vertex* pVertices = new Vertex[pMesh->mNumVertices];
				for (unsigned int j = 0; j < pMesh->mNumVertices; ++j)
				{
					pVertices[j] = Vertex(aiV3ToCraze(pMesh->mVertices[j]), aiV3ToCraze(pMesh->mNormals[j]), aiV3ToCrazeV2(pMesh->mTextureCoords[0][j]));
				}

				unsigned short* pIndices = new unsigned short[pMesh->mNumFaces * 3];
				
				for (unsigned int j = 0; j < pMesh->mNumFaces; ++j)
				{
					pIndices[j * 3 + 0] = pMesh->mFaces[j].mIndices[0];
					pIndices[j * 3 + 1] = pMesh->mFaces[j].mIndices[1];
					pIndices[j * 3 + 2] = pMesh->mFaces[j].mIndices[2];
				}

				std::shared_ptr<MeshData> pMeshData(CrNew MeshData());
				std::string name = pMesh->mName.data;

				char buf[30];

				if (name.empty())
				{
					sprintf_s(buf, 20, "mesh%.3d", i);
					name = buf;
				}

				pMeshData->SetName(name);

				pMeshData->SetIndices(pIndices, pMesh->mNumFaces * 3);
				pMeshData->SetVertices(pVertices, pMesh->mNumVertices);

				MaterialData mat;
				mat.type = MT_NORMAL;

				const aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];

				aiString texture;
				pMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture);
				mat.decalFileName = texture.data;

				pMaterial->Get(AI_MATKEY_SHININESS, mat.specularFactor);

                pMeshData->m_Material = mat;
				res.push_back(pMeshData);

				delete [] pIndices;
				delete [] pVertices;
			}
		}
	}

	return res;
}
