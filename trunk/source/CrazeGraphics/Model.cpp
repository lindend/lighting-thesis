#include "CrazeGraphicsPCH.h"
#include "Model.h"
#include "Geometry/MeshData.h"
#include "Resource/FileDataLoader.h"

#include "MathTypes.h"
#include <iostream>
#include <sstream>

using namespace Craze;
using namespace Craze::Graphics2;

CRAZE_POOL_ALLOC_IMPL(Model);

bool ModelResourceHandler::readComplete(ResourceLoadData* loadData)
{
	std::shared_ptr<Model> model = std::dynamic_pointer_cast<Model>(loadData->res);
	assert(model);

	return model->createFromMemory(loadData->data, loadData->dataSize);
}

template <typename T> bool mread(const char*& pMem, u32& len, T& out)
{
	if (len > sizeof(T))
	{
		out = *(T*)pMem;
		len -= sizeof(T);
		pMem += sizeof(T);

		return true;
	}
	return false;
}

enum MESHENTRYTYPE
{
	MESHENTRY_INTEGRATED,
	MESHENTRY_EXTERNAL,
};

enum MATERIALFLAGS
{
	MATFLAG_HASDECAL = 0x1,
	MATFLAG_HASBUMP = 0x2,
};

template <typename T> void write(std::ofstream& fs, T v)
{
	fs.write((const char*)&v, sizeof(T));
}

bool Craze::Graphics2::saveModel(std::string fileName, std::vector<std::shared_ptr<MeshData>> meshes)
{
	std::ofstream fs;
	fs.open(fileName.c_str(), std::fstream::binary);

	write(fs, (u32)meshes.size());

	for (int i = 0; i < meshes.size(); ++i)
	{
		write(fs, (u32)0);

		write(fs, meshes[i]->getFileSize());
		meshes[i]->writeToFile(fs);

		MaterialData matData = meshes[i]->m_Material;
		u32 flags = 0;
		flags |= matData.decalFileName != "" ? MATFLAG_HASDECAL : 0;
		flags |= matData.bumpFileName != "" ? MATFLAG_HASBUMP : 0;

		write(fs, flags);

		write(fs, matData.decalId);
		write(fs, matData.bumpId);

		write(fs, (u32)0xFFFFFFFF);
	}
	return true;
}

void Model::onDestroy()
{
	for(auto i = m_meshes.begin(); i != m_meshes.end(); ++i)
	{
		delete i->mesh;
	}
}

bool Model::createFromMemory(const char* data, unsigned int size)
{
	/*
	File format of a model:

	u32 number of meshes

	For each mesh:
	u32 type of mesh entry (0 = integrated, 1 = external)
	~If integrated:
	u32 data size
	|mesh data blob|
	~If external
	u64 mesh resource id
	//Material info
	u32 flags (have decal, bump, etc)
	u64 decal id
	u64 bump id
	u32 color


	*/
	const char* cursor = data;

	u32 numMeshes;
	if (!mread(data, size, numMeshes))
	{
		return false;
	}

	for (u32 i = 0; i < numMeshes; ++i)
	{
		u32 type;
		if (!mread(data, size, type))
		{
			return false;
		}

		MeshItem mi;

		if (type == MESHENTRY_INTEGRATED)
		{
			u32 meshLen;
			if (!mread(data, size, meshLen))
			{
				return false;
			}

			Mesh* mesh = CrNew Mesh(gpDevice);

			if (!mesh->createFromMemory(data, meshLen))
			{
				delete mesh;
				return false;
			}

			mi.mesh = mesh;

			data += meshLen;
			size -= meshLen;
		} else if(type == MESHENTRY_EXTERNAL)
		{
			//Not supported yet...
			return false;
		} else
		{
			//Corrupt data, or something
			return false;
		}

		u32 flags;
		if (!mread(data, size, flags))
		{
			return false;
		}

		u64 decalId, bumpId;
		if (!mread(data, size, decalId) || !mread(data, size, bumpId))
		{
			return false;
		}

		if (flags & MATFLAG_HASDECAL)
		{
			std::stringstream decalName;
			decalName << decalId << ".jpg";
			mi.material.m_decal = std::dynamic_pointer_cast<const TextureResource>(gResMgr.loadResource(gFileDataLoader.addFile(decalName.str())));
			if (mi.material.m_decal == NULL)
			{
				mi.material.m_decal = std::dynamic_pointer_cast<const TextureResource>(gResMgr.loadResource(gFileDataLoader.addFile("textureerror.png")));;
			}
		}

		if (flags & MATFLAG_HASBUMP)
		{
			mi.material.m_bump = std::dynamic_pointer_cast<const TextureResource>(gResMgr.loadResource(bumpId));
		}

		u32 color;
		mread(data, size, color);

		m_meshes.push_back(mi);
	}

	return true;
}
