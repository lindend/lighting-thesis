#include "CrazeGraphicsPCH.h"
#include "Scene.h"

#include <assert.h>

#include "Intersection/Intersection.h"
#include "Intersection/BoundingBox.h"

#include "../Light/Light.h"
#include "../DrawList.h"
#include "../Model.h"

//#include "boost/thread.hpp"

using namespace Craze;
using namespace Craze::Graphics2;
Scene::Scene(Device* device) : m_device(device), m_numPointLights(0), m_hlightPool(sizeof(u32)), m_numDirLights(0), m_numSpotLights(0)
{
	m_meshBBs = (BoundingBox*)gMemory.AllocateAligned(16, sizeof(BoundingBox) * MaxMeshesInScene, __FILE__, __LINE__);;
	m_bounds = CrNew BoundingBox();
	m_camera = CrNew Camera();

}
Scene::~Scene()
{
	clearLights();

	gMemory.FreeAligned(m_meshBBs);
	delete m_bounds;
	delete m_camera;

	for (auto i = m_models.begin(); i != m_models.end(); ++i)
	{
		delete (*i);
	}
}

template<typename T> void Scene::freeAllLights(T lights, int numLights)
{
	for (int i = 0; i < numLights; ++i)
	{
		m_hlightPool.free(lights[i].handle);
	}
}

void Scene::clearLights()
{
	freeAllLights(m_pointLights, m_numPointLights);
	freeAllLights(m_spotLights, m_numSpotLights);

	m_numPointLights = 0;
	m_numSpotLights = 0;
	m_numDirLights = 0;
}

HLIGHT Scene::addLight(const PointLight& l)
{
	assert(m_numPointLights != MaxPointLights);
	HLIGHT hlight = (HLIGHT)m_hlightPool.malloc();
	*hlight = m_numPointLights++;
	m_pointLights[*hlight].light = l;
	m_pointLights[*hlight].handle = hlight;
	return hlight;
}

PointLight* Scene::getPointLight(HLIGHT light)
{
	return &m_pointLights[*light].light;
}

void Scene::removePointLight(HLIGHT light)
{
	m_pointLights[*light] = m_pointLights[m_numPointLights - 1];
	*m_pointLights[*light].handle = *light;
	m_hlightPool.free(light);
	--m_numPointLights;
}

HLIGHT Scene::addLight(const SpotLight& l)
{
	assert(m_numSpotLights != MaxSpotLights);
	HLIGHT hlight = (HLIGHT)m_hlightPool.malloc();
	*hlight = m_numSpotLights++;
	m_spotLights[*hlight].light = l;
	m_spotLights[*hlight].handle = hlight;
	return hlight;
}

SpotLight* Scene::getSpotLight(HLIGHT light)
{
	return &m_spotLights[*light].light;
}

void Scene::remoteSpotLight(HLIGHT light)
{
	m_spotLights[*light] = m_spotLights[m_numSpotLights - 1];
	*m_spotLights[*light].handle = *light;
	m_hlightPool.free(light);
	--m_numSpotLights;
}

HLIGHT Scene::addLight(const DirectionalLight& l)
{
	assert(m_numDirLights != MaxDirectionalLights);
	HLIGHT hlight = (HLIGHT)m_hlightPool.malloc();
	*hlight = m_numDirLights++;
	m_dirLights[*hlight].light = l;
	m_dirLights[*hlight].handle = hlight;
	return hlight;
}
DirectionalLight* Scene::getDirectionalLight(HLIGHT light)
{
	return &m_dirLights[*light].light;
}

ModelNode* Scene::addModel(std::shared_ptr<const Model> model, NODEFLAGS flags)
{
	ModelNode* modelNode = CrNew ModelNode(m_meshes.size(), model->getMeshes().size());
    m_models.push_back(modelNode);
	for (auto i = model->getMeshes().begin(); i != model->getMeshes().end(); ++i)
	{
        m_meshes.push_back(*i);
	}
	return modelNode;
}

void Scene::removeModel(ModelNode* model)
{
	//@todo
}

void Scene::update()
{
	for (auto i = m_models.begin(); i != m_models.end(); ++i)
	{
		ModelNode* m = *i;
		if (m->isDirty())
		{
			const int last = m->getFirstMesh() + m->getNumMeshes();
			for (int j = m->getFirstMesh(); j < last; ++j)
			{
				m_meshBBs[j] = m_meshes[j].mesh->getBoundingBox();
			}
			if (m->getNumMeshes() > 0)
			{
				TransformV4(m->getTransform(), (Vector4*)&m_meshBBs[m->getFirstMesh()], 2 * m->getNumMeshes());
			}

			m->unflagDirty();
		}
	}
}

void Scene::buildDrawList(DrawList* drawList, const Matrix4& viewProj) const
{
	PROFILEF();

	_declspec(align(16)) float depth;

	for (u32 i = 0; i < m_models.size(); ++i)
	{
	    const ModelNode* model = m_models[i];

	    if (Intersection::IsInside(viewProj, model->getBoundingBox(), &depth))
	    {
            const u32 maxIdx = model->getNumMeshes() + model->getFirstMesh();

            for (u32 j = 0; j < maxIdx; ++j)
            {
                if (Intersection::IsInside(viewProj, m_meshBBs[j], &depth) || true)
                {
                    const MeshItem& mi = m_meshes[j];
					//if (mi.material.m_decal.get() != NULL)
					{
						drawList->add(RT_MESH, (u32)(depth * (2 << 24)), mi.mesh, &mi.material, &model->getTransform());
					}
                }
            }
        }
	}

	{
		PROFILE("Craze::Graphics2::Scene::CreateDrawList sorting");
		drawList->sort();
	}
}

bool isLightVisible(const Matrix4& viewProj, const DirectionalLight& l)
{
	return true;
}

bool isLightVisible(const Matrix4& viewProj, const SpotLight& l)
{
	return true;
}

bool isLightVisible(const Matrix4& viewProj, const PointLight& l)
{
	return true;
}


const PointLightArray Scene::getVisiblePointLights(const Matrix4& viewProj) const
{
	PROFILEF();

	PointLightArray out;
	out.pointLights = (PointLight*)Next16ByteAddr(gMemory.FrameAlloc(sizeof(PointLight) * m_numPointLights + 15));
	out.numLights = 0;

	int outIdx = 0;
	for (int i = 0; i < m_numPointLights; ++i)
	{
		if (isLightVisible(viewProj, m_pointLights[i].light))
		{
			out.pointLights[outIdx] = m_pointLights[i].light;
			++outIdx;
		}
	}

	out.numLights = outIdx;

	return out;
}

const SpotLightArray Scene::getVisibleSpotLights(const Matrix4& viewProj) const
{
	PROFILEF();

	SpotLightArray out;
	out.spotLights = (SpotLight*)Next16ByteAddr(gMemory.FrameAlloc(sizeof(SpotLight) * m_numSpotLights + 15));
	out.numLights = 0;

	int outIdx = 0;
	for (int i = 0; i < m_numSpotLights; ++i)
	{
		if (isLightVisible(viewProj, m_spotLights[i].light))
		{
			out.spotLights[outIdx] = m_spotLights[i].light;
			++outIdx;
		}
	}
	out.numLights = outIdx;
	return out;
}

const DirectionalLight* Scene::getDirectionalLights(int& numLights) const
{
	numLights = m_numDirLights;
	
	DirectionalLight* lights = (DirectionalLight*)Next16ByteAddr(gMemory.FrameAlloc(sizeof(DirectionalLight) * m_numDirLights + 15));
	for (int i = 0; i < m_numDirLights; ++i)
	{
		lights[i] = m_dirLights[i].light;
	}
	return lights;
}