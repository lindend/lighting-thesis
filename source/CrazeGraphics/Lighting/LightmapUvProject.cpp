#include "CrazeGraphicsPCH.h"
#include "LightmapUvProject.h"

#include "V3Math.h"

#include "../Geometry/MeshData.h"
#include "../kdTree/kdTree.h"

using namespace Craze;
using namespace Craze::Graphics2;

inline const Vector3 calcNormal(const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
    return Normalize(Cross(v2 - v0, v1 - v0));
}
inline const Vertex fromBar(const Vector3& bar, const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
    Vertex res;
    res.position = bar.x * v0.position + bar.y * v1.position + bar.z * v2.position;
    res.uv = bar.x * v0.uv + bar.y * v1.uv + bar.z * v2.uv;
    return res;
}
const Vertex closestPointOnTri(const Vector3& point, const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
    //Code from Real-time collision detection, chapter 5.1.4
    const Vector3 ab = v1.position - v0.position;
    const Vector3 ac = v2.position - v0.position;
    const Vector3 ap = point - v0.position;

    const float d1 = Dot(ab, ap);
    const float d2 = Dot(ac, ap);
    if (d1 <= 0.f && d2 <= 0.f) return v0;

    const Vector3 bp = point - v1.position;
    const float d3 = Dot(ab, bp);
    const float d4 = Dot(ac, bp);
    if (d3 >= 0.f && d4 <= 0.f) return v1;

    const float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
    {
        const float v = d1 / (d1 -d3);
        return fromBar(Vector3(1.f - v, v, 0.f), v0, v1, v2);
    }

    const Vector3 cp = point - v2.position;
    const float d5 = Dot(ab, cp);
    const float d6 = Dot(ac, cp);
    if (d6 >= 0.f && d5 <= d6) return v2;

    const float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
    {
        const float w = d2 / (d2 - d6);
        return fromBar(Vector3(0.f, 1.f - w, w), v0, v1, v2);
    }

	const float va = d3 * d6 - d5 * d4;
	if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)
	{
		const float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return fromBar(Vector3(0.f, 1.f - w, w), v0, v1, v2);
	}

    const float denom = 1.f / (va + vb + vc);
    const float v = vb * denom;
    const float w = vc * denom;
    return fromBar(Vector3(1.f - v - w, v, w), v0, v1, v2);
}

//Should also check for points in triangles facing in the same normal as pPoint
Vertex findClosestVertOnMesh(const Vertex* pVerts, const unsigned short* pTris, const unsigned int numTris, const Vertex& point, float& minDist)
{
    assert(numTris > 0);

    //Just initialize the return value to the first vertex
    minDist = LengthSquared(pVerts->position - point.position);
    Vertex ret = *pVerts;
    for (unsigned int i = 0; i < numTris; ++i)
    {
        //const UvTriangle& t = pTris[i];
        //const Vertex v0 = { t.v0, Vector3::ZERO, t.uv0 };
        //const Vertex v1 = { t.v1, Vector3::ZERO, t.uv1 };
        //const Vertex v2 = { t.v2, Vector3::ZERO, t.uv2 };
        const unsigned short v0 = pTris[i * 3 + 0];
        const unsigned short v1 = pTris[i * 3 + 1];
        const unsigned short v2 = pTris[i * 3 + 2];

        const Vertex tst = closestPointOnTri(point.position, pVerts[v0], pVerts[v1], pVerts[v2]);

		const float tstDist = Length(tst.position - point.position);

        if (tstDist < minDist)
        {
            ret = tst;
            minDist = tstDist;
        }
    }
    return ret;
	/*if (pKd->IsLeaf())
	{
	    minDist = -1.f;
		const int triIdx = pKd->triIndex;
		const int lastIdx = triIdx + pKd->GetNumTris();

		for (int i = triIdx; i < lastIdx; ++i)
		{
            Vector3 normal = calcNormal(pTris->GetV0(), pTris->GetV1(), pTris->GetV2());
            if (Dot(normal, pPoint->normal) < 0.f)
            {
                continue;
            }
            //Must have somewhere to store the result and to check for min distance

		}
	} else
	{

	}*/
}


void Craze::Graphics2::projectLightmapLowToHigh(std::shared_ptr<MeshData> lowPoly, std::shared_ptr<MeshData> highPoly)
{
	/*kdTree<UvTriangle> kd;
	kd.Create(lowPoly);

	unsigned int numNodes;
	const kdNode* pNodes = kd.GetKdNodes(numNodes);
	unsigned int numTris;
	const UvTriangle* pTris = kd.GetTriangles(numTris);
*/
    const Vertex* pLPVerts = lowPoly->GetPosNormalUv();
    const unsigned short* pLPTris = lowPoly->GetIndices();
    const int numLPTris = lowPoly->GetNumIndices() / 3;

	const int numVerts = highPoly->GetNumVertices();
	const Vertex* pVerts = highPoly->GetPosNormalUv();
	LightMapVertex* pLMVerts = new LightMapVertex[numVerts];

	for (int i = 0; i < numVerts; ++i)
	{
		const Vertex& vert = pVerts[i];
        float dist;
		pLMVerts[i] = LightMapVertex(findClosestVertOnMesh(pLPVerts, pLPTris, numLPTris, vert, dist).uv);
	}

	highPoly->SetVertices(pLMVerts, numVerts);

	delete [] pLMVerts;
}
