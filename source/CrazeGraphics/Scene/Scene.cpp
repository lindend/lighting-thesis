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
Scene::Scene(Device* device) : m_device(device), m_numLights(0), m_maxLightIndex(-1)
{
	m_meshBBs = (BoundingBox*)gMemory.AllocateAligned(16, sizeof(BoundingBox) * MaxMeshesInScene, __FILE__, __LINE__);;
	m_bounds = CrNew BoundingBox();
	m_camera = CrNew Camera();

	memset(m_activeLights, 0, sizeof(m_activeLights));
}
Scene::~Scene()
{
	gMemory.FreeAligned(m_meshBBs);
	delete m_bounds;
	delete m_camera;

	for (auto i = m_models.begin(); i != m_models.end(); ++i)
	{
		delete (*i);
	}
}

Light* Scene::addLight(const Light& l)
{
	PROFILEF();

	if (m_numLights >= MaxLightsInScene)
	{
		return nullptr;
	}

	int idx = 0;
	if (m_maxLightIndex + 1 == m_numLights)
	{
		idx = m_numLights;
	} else
	{
		idx = findFirstFreeLightSlot();
	}
	m_maxLightIndex = Max(m_maxLightIndex, idx);
	++m_numLights;
	m_lights[idx] = l;
	m_activeLights[idx / 32] |= 1 << idx % 32;
	return &m_lights[idx];
}

void Scene::removeLight(Light* pLight)
{
	PROFILEF();

	int idx = (pLight - m_lights) / sizeof(Light);

	assert(idx >= 0 && idx < MaxLightsInScene);

	if (idx == m_maxLightIndex)
	{
		--m_maxLightIndex;
	}

	m_activeLights[idx / 32] &= ~(1 << idx % 32);
}

inline int firstBitNotSet(int v)
{
	//http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
	static const int MultiplyDeBruijnBitPosition[32] =
	{
	  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
	  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return MultiplyDeBruijnBitPosition[((unsigned __int32)((v & -v) * 0x077CB531U)) >> 27];
}

int Scene::findFirstFreeLightSlot() const
{
	assert(m_numLights < MaxLightsInScene);

	int i = 0;
	for ( ; m_activeLights[i] == 0xFFFFFFFF && i < MaxLightsInScene; ++i);

	assert(m_activeLights[i] != 0xFFFFFFFF);

	return i * 32 + firstBitNotSet(m_activeLights[i]);
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

bool isLightVisible(const Matrix4& viewProj, const Light& l)
{
	return true;
}

LightArray Scene::getVisibleLights(const Matrix4& viewProj)
{
	PROFILEF();
	int numSoAs = m_numLights / 4 + Min<int>(1, m_numLights % 4);

	LightArray out;

	out.pPositions = (SoAV3*)Next16ByteAddr(gMemory.FrameAlloc(sizeof(SoAV3) * numSoAs + 15));
	out.pColors = (Vector4*)Next16ByteAddr(gMemory.FrameAlloc(sizeof(Vector4) * m_numLights + 15));
	out.pRanges = (float*)Next16ByteAddr(gMemory.FrameAlloc(sizeof(float) * m_numLights + 15));
	out.numPosSoA = numSoAs;

	int n = 0;
	int outIdx = 0;
	int i = 0;
	for (; i <= m_maxLightIndex && n < m_numLights; ++i)
	{
		if (m_activeLights[i / 32] == 0xFFFFFFFF || m_activeLights[i / 32] & (1 << (i % 32)))
		{
			++n;
			Light l = m_lights[i];
			if (isLightVisible(viewProj, l))
			{
				out.pPositions[outIdx / 4].xs.m128_f32[outIdx % 4] = l.pos->x;
				out.pPositions[outIdx / 4].ys.m128_f32[outIdx % 4] = l.pos->y;
				out.pPositions[outIdx / 4].zs.m128_f32[outIdx % 4] = l.pos->z;

				out.pRanges[outIdx] = l.pos->w;
				out.pColors[outIdx] = l.color;

				++outIdx;
			}
		}
	}
	m_maxLightIndex = i - 1;

	out.numLights = outIdx;

	for (; outIdx % 4; ++outIdx)
	{
		out.pPositions[outIdx / 4].xs.m128_f32[outIdx % 4] = 0.f;
		out.pPositions[outIdx / 4].ys.m128_f32[outIdx % 4] = 0.f;
		out.pPositions[outIdx / 4].zs.m128_f32[outIdx % 4] = 0.f;
	}

	return out;
}
