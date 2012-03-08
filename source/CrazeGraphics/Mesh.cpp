#include "CrazeGraphicsPCH.h"
#include "Mesh.h"

#include "Intersection/BoundingBox.h"

#include "Device.h"
#include "Material.h"
#include "Geometry/MeshData.h"
#include "Buffer/Buffer.h"

using namespace Craze;
using namespace Craze::Graphics2;

CRAZE_POOL_ALLOC_IMPL(Mesh);

Mesh::Mesh(Device* device)
{
	m_device = device;
	m_boundingBox = CrNew BoundingBox();
}

bool MeshResourceHandler::readComplete(ResourceLoadData* data)
{
	Mesh* res = dynamic_cast<Mesh*>(data->res);

	if (res)
	{
		return res->createFromMemory(data->data, data->dataSize);
	}

	return false;
}
/*


			int numSubs = (!matData.decalFileName.empty() ? 1 : 0) + (!matData.bumpFileName.empty() ? 1 : 0);
			const Resource** subs = (numSubs > 0 ? new const Resource*[numSubs] : nullptr);

			res->m_decalIndex = res->m_bumpIndex = -1;

			if (!matData.decalFileName.empty())
			{
				res->m_decalIndex = 0;
				subs[0] = gResMgr.loadResource(path + matData.decalFileName);
			}

			if (!matData.bumpFileName.empty())
			{
				res->m_bumpIndex = (numSubs == 1 ? 0 : 1);
				subs[res->m_bumpIndex] = gResMgr.loadResource(path + matData.bumpFileName);
			}

			res->setDependencies(subs, numSubs);

			return true;
bool MeshResourceHandler::allComplete(ResourceLoadData* data)
{
	MeshResource* res = dynamic_cast<MeshResource*>(data->res);
	assert(res != nullptr);

	Material* pMat = res->m_mesh->GetMaterial();

	if (res->m_decalIndex >= 0)
	{
		TextureResource* pDecal = res->getDependencies()[res->m_decalIndex]->GetResourceData<TextureResource>();

		if (!pDecal)
		{
			return false;
		}

		pMat->m_Decal = pDecal->m_texture;
	}

	if (res->m_bumpIndex >= 0)
	{
		TextureResource* pBump = res->GetDependencies()[res->m_bumpIndex]->GetResourceData<TextureResource>();

		if (!pBump)
		{
			return false;
		}

		pMat->m_Bump = pBump->m_texture;
	}

	return true;
}
*/

void Mesh::setIndexData(const unsigned short* indices, unsigned int numIndices)
{
	m_indices = GeometryBuffer::Create(m_device, GeometryBuffer::INDEX, indices, 2, numIndices, false, "Mesh index buffer");
}

void Mesh::setVertexData(const Vertex* vertices, unsigned int numVertices)
{
	BoundingBox bb;
	for (unsigned int i = 0; i < numVertices; ++i)
	{
		bb.IncludePoint(vertices[i].position);
	}

	*m_boundingBox = bb;

	m_vertexStream.SetStreamPosNormalUv(GeometryBuffer::Create(m_device, GeometryBuffer::VERTEX, vertices, sizeof(Vertex), numVertices, false, "Mesh vertex buffer"));
}

void Mesh::setVertexData(const SkinnedVertex* vertices, unsigned int numVertices)
{
	m_vertexStream.SetStreamSkinning(GeometryBuffer::Create(m_device, GeometryBuffer::VERTEX, vertices, sizeof(SkinnedVertex), numVertices, false, "Mesh skinned buffer"));
}

void Mesh::setVertexData(const LightMapVertex* vertices, unsigned int numVertices)
{
	m_vertexStream.SetStreamLightMap(GeometryBuffer::Create(m_device, GeometryBuffer::VERTEX, vertices, sizeof(LightMapVertex), numVertices, false, "Mesh skinned buffer"));
}

void Mesh::draw() const
{
	m_device->SetVertexStreams(&m_vertexStream);
	m_device->SetIndexBuffer(m_indices);
	m_device->DrawIndexed();
}

bool Mesh::createFromMemory(const char* data, unsigned long length)
{
	const char* cursor = data;

	const bool is32bitIndices = *(bool*)cursor;
	cursor += sizeof(bool);

	int numIndices = *(int*)cursor;
	cursor += sizeof(int);
	assert(cursor - data < (long)length);

	m_meshData.reset(CrNew MeshData());

	if (is32bitIndices)
	{
		LOG_CRITICAL("Unsupported index format. Annoy the developer a bit so that he implements this functionality");
		return false;

	} else
	{
		const unsigned short* indices = (const unsigned short*)cursor;
		setIndexData(indices, numIndices);
		cursor += sizeof(unsigned short) * numIndices;
		assert(cursor - data < (long)length);
		m_meshData->SetIndices(indices, numIndices);
	}

	const int numVerts = *(int*)cursor;
	cursor += sizeof(int);
	assert(cursor - data < (long)length);

	if (numVerts > 0)
	{
		const Vertex* verts = (const Vertex*)cursor;
		setVertexData(verts, numVerts);
		m_meshData->SetVertices(verts, numVerts);
		cursor += sizeof(Vertex) * numVerts;
		assert(cursor - data < (long)length);
	}



	const int numLMVerts = *(int*)cursor;
	cursor += sizeof(int);
	assert(cursor - data < (long)length);

	if (numLMVerts > 0)
	{
		const LightMapVertex* lmVerts = (const LightMapVertex*)cursor;
		setVertexData(lmVerts, numLMVerts);
		m_meshData->SetVertices(lmVerts, numLMVerts);
		cursor += sizeof(LightMapVertex) * numLMVerts;
		assert(cursor - data < (long)length);
	}

	const int numSkinnedVerts = *(int*)cursor;
	cursor += sizeof(int);
	assert(cursor - data <= (long)length);

	if (numSkinnedVerts > 0)
	{
		const SkinnedVertex* skVerts = (const SkinnedVertex*)cursor;
		setVertexData(skVerts, numSkinnedVerts);
		m_meshData->SetVertices(skVerts, numSkinnedVerts);
		cursor += sizeof(SkinnedVertex) * numSkinnedVerts;
	}

	return true;
}

std::shared_ptr<Mesh> Mesh::createFromData(Device* device, MeshData* data)
{
	assert(data != nullptr);

	std::shared_ptr<Mesh> mesh(CrNew Mesh(device));
	assert(mesh);

	mesh->setVertexData(data->GetPosNormalUv(), data->GetNumVertices());
	mesh->setVertexData(data->GetSkinned(), data->GetNumVertices());
	mesh->setVertexData(data->GetLightMapUv(), data->GetNumVertices());
	mesh->setIndexData(data->GetIndices(), data->GetNumIndices());

	return mesh;
}

std::shared_ptr<Mesh> Mesh::createScreenQuad(Device* pDevice)
{
	Vertex vertices[4];
	unsigned short indices[6];

	vertices[0] = Vertex(Vector3(-1.0f, -1.0f, 1.0f), Vector3::BACKWARD, Vector2(0.0f, 0.0f));
	vertices[1] = Vertex(Vector3( 1.0f, -1.0f, 1.0f), Vector3::BACKWARD, Vector2(1.0f, 0.0f));
	vertices[2] = Vertex(Vector3( 1.0f,  1.0f, 1.0f), Vector3::BACKWARD, Vector2(1.0f, 1.0f));
	vertices[3] = Vertex(Vector3(-1.0f,  1.0f, 1.0f), Vector3::BACKWARD, Vector2(0.0f, 1.0f));

	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 0;
	indices[4] = 3;
	indices[5] = 2;

	std::shared_ptr<Mesh> pMesh(CrNew Mesh(pDevice));
	pMesh->setVertexData(vertices, 4);
	pMesh->setIndexData(indices, 6);
	return pMesh;
}
/*
std::shared_ptr<Mesh> Mesh::CreateCube(Device* pDevice, float width, float height, float depth)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;

	float zOffs = depth / 2.0f;
	float yOffs = height / 2.0f;
	float xOffs = width / 2.0f;

	//Right
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, -zOffs),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, -zOffs),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.0f, 0.5f)));
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, zOffs),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.5f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, zOffs),		Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.5f, 0.5f)));

	//Left
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, zOffs),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.0f, 0.5f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, zOffs),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, -zOffs),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.5f, 0.5f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, -zOffs),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.5f, 1.0f)));

	//Up
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, -zOffs),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, zOffs),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, -zOffs),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, zOffs),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(1.0f, 1.0f)));

	//Down
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, -zOffs),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, zOffs),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, -zOffs),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, zOffs),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(1.0f, 1.0f)));

	//Back
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, zOffs),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(0.5f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, zOffs),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(0.5f, 0.5f)));
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, zOffs),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, zOffs),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(1.0f, 0.5f)));

	//Front
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, -zOffs),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(0.5f, 0.5f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, -zOffs),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(0.5f, 1.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, -zOffs),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(1.0f, 0.5f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, -zOffs),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(1.0f, 1.0f)));

	/-* old texture coords....

	//Right
	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, -zOffs, 1.0f),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, -zOffs, 1.0f),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, zOffs, 1.0f),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, zOffs, 1.0f),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(1.0f, 1.0f)));

	//Left
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, zOffs, 1.0f),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, zOffs, 1.0f),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, -zOffs, 1.0f),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, -zOffs, 1.0f),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(1.0f, 1.0f)));

	//Up
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, -zOffs, 1.0f),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, zOffs, 1.0f),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, -zOffs, 1.0f),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, zOffs, 1.0f),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(1.0f, 1.0f)));

	//Down
	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, -zOffs, 1.0f),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, zOffs, 1.0f),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, -zOffs, 1.0f),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, zOffs, 1.0f),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(1.0f, 1.0f)));

	//Back
	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, zOffs, 1.0f),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, zOffs, 1.0f),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, zOffs, 1.0f),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, zOffs, 1.0f),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(1.0f, 1.0f)));

	//Front
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, -zOffs, 1.0f),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, -zOffs, 1.0f),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, -zOffs, 1.0f),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, -zOffs, 1.0f),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(1.0f, 1.0f)));

	*-/


	for(int i = 0; i < 6; ++i)
	{
		indices.push_back(i * 4 + 0);
		indices.push_back(i * 4 + 1);
		indices.push_back(i * 4 + 2);

		indices.push_back(i * 4 + 1);
		indices.push_back(i * 4 + 3);
		indices.push_back(i * 4 + 2);
	}

	std::shared_ptr<Mesh> pMesh(CrNew Mesh(pDevice));
	pMesh->SetVertexData(&vertices[0], vertices.size());
	pMesh->SetIndexData(&indices[0], indices.size());

	pMesh->m_pMeshData.reset(CrNew MeshData());
	pMesh->m_pMeshData->SetIndices(&indices[0], indices.size());
	pMesh->m_pMeshData->SetVertices(&vertices[0], vertices.size());

	return pMesh;
}

std::shared_ptr<Mesh> Mesh::CreateInvertedCube(Device* pDevice, float width, float height, float depth)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;

	float zOffs = depth / 2.0f;
	float yOffs = height / 2.0f;
	float xOffs = width / 2.0f;

	//Right
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, -zOffs),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, -zOffs),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.0f, 0.5f)));
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, zOffs),	Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.5f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, zOffs),		Vector3(1.0f, 0.0f, 0.0f),		Vector2(0.5f, 0.5f)));

	//Left
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, zOffs),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.0f, 0.5f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, zOffs),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, -zOffs),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.5f, 0.5f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, -zOffs),	Vector3(-1.0f, 0.0f, 0.0f),		Vector2(0.5f, 1.0f)));

	//Up
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, -zOffs),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, zOffs),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, -zOffs),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, zOffs),	Vector3(0.0f, 1.0f, 0.0f),		Vector2(1.0f, 1.0f)));

	//Down
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, -zOffs),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, zOffs),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, -zOffs),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, zOffs),	Vector3(0.0f, -1.0f, 0.0f),		Vector2(1.0f, 1.0f)));

	//Back
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, zOffs),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(0.5f, 0.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, zOffs),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(0.5f, 0.5f)));
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, zOffs),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(1.0f, 0.0f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, zOffs),	Vector3(0.0f, 0.0f, 1.0f),		Vector2(1.0f, 0.5f)));

	//Front
	vertices.push_back(Vertex(Vector3(-xOffs, -yOffs, -zOffs),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(0.5f, 0.5f)));
	vertices.push_back(Vertex(Vector3(-xOffs, yOffs, -zOffs),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(0.5f, 1.0f)));
	vertices.push_back(Vertex(Vector3(xOffs, -yOffs, -zOffs),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(1.0f, 0.5f)));
	vertices.push_back(Vertex(Vector3(xOffs, yOffs, -zOffs),	Vector3(0.0f, 0.0f, -1.0f),		Vector2(1.0f, 1.0f)));

	for (int i = 0; i < 6; ++i)
	{
		indices.push_back(i * 4 + 0);
		indices.push_back(i * 4 + 2);
		indices.push_back(i * 4 + 1);

		indices.push_back(i * 4 + 1);
		indices.push_back(i * 4 + 2);
		indices.push_back(i * 4 + 3);
	}

	std::shared_ptr<Mesh> pMesh(CrNew Mesh(pDevice));
	pMesh->SetVertexData(&vertices[0], vertices.size());
	pMesh->SetIndexData(&indices[0], indices.size());

	pMesh->m_pMeshData.reset(CrNew MeshData());
	pMesh->m_pMeshData->SetIndices(&indices[0], indices.size());
	pMesh->m_pMeshData->SetVertices(&vertices[0], vertices.size());

	return pMesh;
}*/
/*


std::shared_ptr<Mesh> Mesh::CreateInvertedCube(Device* pDevice, float width, float height, float depth)
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	float zOffs = depth / 2.0f;
	float yOffs = height / 2.0f;
	float xOffs = width / 2.0f;

	float nScale = -0.577350269189626f;

	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, zOffs, 1.0f), Vector3(nScale, -nScale, nScale), Vector2(0.0f, 0.5f)));
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, zOffs, 1.0f), Vector3(-nScale, -nScale, nScale), Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, zOffs, 1.0f), Vector3(nScale, nScale, nScale), Vector2(1.0f, 0.5f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, zOffs, 1.0f), Vector3(-nScale, nScale, nScale), Vector2(1.0f, 0.0f)));

	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, -zOffs, 1.0f), Vector3(nScale, -nScale, -nScale), Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, -zOffs, 1.0f), Vector3(-nScale, -nScale, -nScale), Vector2(0.0f, 0.5f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, -zOffs, 1.0f), Vector3(nScale, nScale, -nScale), Vector2(1.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, -zOffs, 1.0f), Vector3(-nScale, nScale, -nScale), Vector2(1.0f, 0.5f)));

	//Near
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);

	//Far
	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(7);
	indices.push_back(5);
	indices.push_back(4);
	indices.push_back(6);

	//Top
	indices.push_back(2);
	indices.push_back(7);
	indices.push_back(6);
	indices.push_back(2);
	indices.push_back(6);
	indices.push_back(7);

	//Bottom
	indices.push_back(1);
	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(1);
	indices.push_back(0);
	indices.push_back(4);

	//Right
	indices.push_back(4);
	indices.push_back(2);
	indices.push_back(6);
	indices.push_back(4);
	indices.push_back(0);
	indices.push_back(2);

	//Left
	indices.push_back(1);
	indices.push_back(7);
	indices.push_back(3);
	indices.push_back(1);
	indices.push_back(5);
	indices.push_back(7);

	std::shared_ptr<Mesh> pMesh(new Mesh(pDevice));
	pMesh->SetVertexData(vertices);
	pMesh->SetIndexData(indices);
	return pMesh;
}

std::shared_ptr<Mesh> Mesh::CreateCube(Device* pDevice, float width, float height, float depth)
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	float zOffs = depth / 2.0f;
	float yOffs = height / 2.0f;
	float xOffs = width / 2.0f;

	float nScale = 0.577350269189626f;

	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, zOffs, 1.0f), Vector3(nScale, -nScale, nScale), Vector2(0.0f, 0.5f)));
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, zOffs, 1.0f), Vector3(-nScale, -nScale, nScale), Vector2(0.0f, 0.0f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, zOffs, 1.0f), Vector3(nScale, nScale, nScale), Vector2(1.0f, 0.5f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, zOffs, 1.0f), Vector3(-nScale, nScale, nScale), Vector2(1.0f, 0.0f)));

	vertices.push_back(Vertex(Vector4(xOffs, -yOffs, -zOffs, 1.0f), Vector3(nScale, -nScale, -nScale), Vector2(0.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, -yOffs, -zOffs, 1.0f), Vector3(-nScale, -nScale, -nScale), Vector2(0.0f, 0.5f)));
	vertices.push_back(Vertex(Vector4(xOffs, yOffs, -zOffs, 1.0f), Vector3(nScale, nScale, -nScale), Vector2(1.0f, 1.0f)));
	vertices.push_back(Vertex(Vector4(-xOffs, yOffs, -zOffs, 1.0f), Vector3(-nScale, nScale, -nScale), Vector2(1.0f, 0.5f)));

	//Near
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(1);

	//Far
	indices.push_back(5);
	indices.push_back(7);
	indices.push_back(6);
	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(4);

	//Top
	indices.push_back(2);
	indices.push_back(6);
	indices.push_back(7);
	indices.push_back(2);
	indices.push_back(7);
	indices.push_back(3);

	//Bottom
	indices.push_back(1);
	indices.push_back(5);
	indices.push_back(4);
	indices.push_back(1);
	indices.push_back(4);
	indices.push_back(0);

	//Right
	indices.push_back(4);
	indices.push_back(6);
	indices.push_back(2);
	indices.push_back(4);
	indices.push_back(2);
	indices.push_back(0);

	//Left
	indices.push_back(1);
	indices.push_back(3);
	indices.push_back(7);
	indices.push_back(1);
	indices.push_back(7);
	indices.push_back(5);

	std::shared_ptr<Mesh> pMesh(new Mesh(pDevice));
	pMesh->SetVertexData(vertices);
	pMesh->SetIndexData(indices);
	return pMesh;
}
*/
