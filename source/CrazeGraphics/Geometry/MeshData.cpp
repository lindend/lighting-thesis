#include "CrazeGraphicsPCH.h"
#include "MeshData.h"

#include "Util/optional.hpp"

#include "EventLogger.h"
#include "VertexStreams.h"

using namespace Craze;
using namespace Craze::Graphics2;


template <typename T> optional<T> mread(const char*& pMem, unsigned long& len)
{
	optional<T> res;
	if (len > sizeof(T))
	{
		res = *(T*)pMem;
		len -= sizeof(T);
		pMem += sizeof(T);

		return res;
	}

	return res;
}

bool mrcopy(void* pDest, unsigned long size, const char*& pMem, unsigned long& len)
{
	if (len >= size)
	{
		memcpy(pDest, pMem, size);
		pMem += size;
		len -= size;

		return true;
	}

	return false;
}


std::shared_ptr<MeshData> Craze::Graphics2::LoadMeshFromMemory(const char* pData, unsigned long length)
{
    std::shared_ptr<MeshData> pMesh(CrNew MeshData());

    pMesh->ParseFile(pData, length);

	return pMesh;
}

bool Craze::Graphics2::SaveMeshToFile(std::shared_ptr<MeshData> pMesh, const std::string& fileName)
{
	FILE* f;
	if (fopen_s(&f, fileName.c_str(), "wb") != 0)
	{
		return false;
	}

	pMesh->WriteToFile(f);

	fclose(f);

	return true;
}

MeshData::~MeshData()
{
	delete [] m_pVertices;
	delete [] m_pLightMapped;
	delete [] m_pSkinned;
	delete [] m_pIndices;
}

void MeshData::SetIndices(const unsigned short* pIndices, int numIndices) __restrict
{
    delete [] m_pIndices;
    m_pIndices = new unsigned short[numIndices];
    memcpy(m_pIndices, pIndices, sizeof(unsigned short) * numIndices);
    m_NumIndices = numIndices;
}

void MeshData::SetVertices(const Vertex* pVertices, int numVertices)
{
	assert(m_NumVertices == 0 || m_NumVertices == numVertices);
    delete [] m_pVertices;
    m_pVertices = new Vertex[numVertices];
    memcpy(m_pVertices, pVertices, sizeof(Vertex) * numVertices);
    m_NumVertices = numVertices;
}

void MeshData::SetVertices(const LightMapVertex* pVertices, int numVertices)
{
	assert(m_NumVertices == 0 || m_NumVertices == numVertices);
    delete [] m_pVertices;
    m_pLightMapped = new LightMapVertex[numVertices];
    memcpy(m_pLightMapped, pVertices, sizeof(LightMapVertex) * numVertices);
    m_NumVertices = numVertices;
}

void MeshData::SetVertices(const SkinnedVertex* pVertices, int numVertices)
{
	assert(m_NumVertices == 0 || m_NumVertices == numVertices);
	delete [] m_pVertices;
	m_pSkinned = new SkinnedVertex[numVertices];
	memcpy(m_pSkinned, pVertices, sizeof(SkinnedVertex) * numVertices);
	m_NumVertices = numVertices;
}

bool MeshData::ParseFile(const char* pData, unsigned long dataLength)
{
    delete [] m_pIndices;
    m_pIndices = 0;

    delete [] m_pVertices;
    m_pVertices = 0;

    //Code for parsing a craze mesh file goes here

	optional<int> oint = mread<int>(pData, dataLength);
	
	if (oint.Valid())
	{
		//This is the mesh type, ignore
	} else
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}

	oint = mread<int>(pData, dataLength);

	if (oint.Valid())
	{
		m_NumIndices = oint.Get();
	} else
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}


    m_pIndices = new unsigned short[m_NumIndices];

	if (!mrcopy(m_pIndices, sizeof(unsigned short) * m_NumIndices, pData, dataLength))
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}
    
	oint = mread<int>(pData, dataLength);

	if (oint.Valid())
	{
		m_NumVertices = oint.Get();
	} else
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}

	unsigned long size = 0;
    m_pVertices = new Vertex[m_NumVertices];
	size = sizeof(Vertex) * m_NumVertices;

	if (!mrcopy(m_pVertices, size, pData, size))
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}

	oint = mread<int>(pData, dataLength);

	if (oint.Valid())
	{
		m_Material.type = (MATERIAL_TYPE)oint.Get();
	} else
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}

	optional<float> oflt = mread<float>(pData, dataLength);
	
	if (oflt.Valid())
	{
		m_Material.specularFactor = oflt.Get();
	} else
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}
	
	int strLength = 0;

	oint = mread<int>(pData, dataLength);

	if (oint.Valid())
	{
		strLength = oint.Get();
	} else
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}

	char* str = new char[strLength + 1];
	
	if (!mrcopy(str, strLength, pData, dataLength))
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}

	str[strLength] = '\0';
	m_Material.decalFileName = str;
	delete [] str;

	oint = mread<int>(pData, dataLength);

	if (oint.Valid())
	{
		strLength = oint.Get();
	} else
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}

	str = new char[strLength + 1];
	
	if (!mrcopy(str, strLength, pData, dataLength))
	{
		LOG_ERROR("Unexpected EOF while loading mesh");
		return false;
	}

	str[strLength] = '\0';
	m_Material.bumpFileName = str;
	delete [] str;

    return true;
}

u32 MeshData::getFileSize()
{
	return 1 + 4 * 4 + 2 * m_NumIndices + sizeof(Vertex) * m_NumVertices;
}

template <typename T> void write(std::ofstream& fs, T v)
{
	fs.write((const char*)&v, sizeof(T));
}

void MeshData::writeToFile(std::ofstream& fs)
{
	write(fs, false);

	write(fs, (u32)m_NumIndices);
	for (int i = 0; i < m_NumIndices; ++i)
	{
		write(fs, (u16)m_pIndices[i]);
	}

	write(fs, (u32)m_NumVertices);
	fs.write((const char*)m_pVertices, sizeof(Vertex) * m_NumVertices);
	
	//No lightmapped vertices
	write(fs, (u32)0);

	//No skinned vertices
	write(fs, (u32)0);
}

/*
	int numIndices = *(int*)cursor;
	const bool is32bitIndices = (numIndices & 0x80000000) != 0;
	numIndices = numIndices & 0x7FFFFFFF;
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
	assert(cursor - data < (long)length);

	if (numSkinnedVerts > 0)
	{
		const SkinnedVertex* skVerts = (const SkinnedVertex*)cursor;
		setVertexData(skVerts, numSkinnedVerts);
		m_meshData->SetVertices(skVerts, numSkinnedVerts);
		cursor += sizeof(SkinnedVertex) * numSkinnedVerts;
	}

	return true;
	*/

void MeshData::WriteToFile(FILE* f)
{
	int zero = 0;
    fwrite(&zero, sizeof(int), 1, f);

    fwrite(&m_NumIndices, sizeof(int), 1, f);
    fwrite(m_pIndices, sizeof(unsigned short), m_NumIndices, f);

    fwrite(&m_NumVertices, sizeof(int), 1, f);
	
	size_t size = sizeof(Vertex);
	fwrite(m_pVertices, size, m_NumVertices, f);

	fwrite(&m_Material.type, sizeof(int), 1, f);

	fwrite(&m_Material.specularFactor, sizeof(float), 1, f);

	int strLength = m_Material.decalFileName.length() + 1;
	fwrite(&strLength, sizeof(int), 1, f);
	fwrite(m_Material.decalFileName.c_str(), sizeof(char), strLength, f);

	strLength = m_Material.bumpFileName.length() + 1;
	fwrite(&strLength, sizeof(int), 1, f);
	fwrite(m_Material.bumpFileName.c_str(), sizeof(char), strLength, f);

}
