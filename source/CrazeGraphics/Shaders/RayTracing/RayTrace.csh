#include "RayTracing/PhotonRay.incl"

#define TRICACHESIZE 128
#define STACKSIZE 3
#define INFINITY 100000.f
#define GROUPSIZE 128

struct Triangle
{
	float3 v0;
	float3 v1;
	float3 v2;
};

struct kdNode
{
	float plane;
	uint flags;
};

struct StackItem
{
	kdNode n;
	float tMin;
	float tMax;
	uint idx;
};
/*
struct ShortStack
{
	StackItem elems[STACKSIZE];
	uint top;
	uint numElems;
};*/

cbuffer KDSceneInfo : register(b1)
{
	float3 SceneBBMin : packoffset(c0);
	uint NumRays :      packoffset(c0.w);	
	float3 SceneBBMax : packoffset(c1);
	
};

AppendStructuredBuffer<PhotonRay> OutRays : register(u0);

StructuredBuffer<Triangle> Triangles : register(t0);
StructuredBuffer<kdNode> KdNodes : register(t1);
StructuredBuffer<PhotonRay> Rays : register(t2);

groupshared Triangle TriCache[TRICACHESIZE];


bool rayAABBTest(PhotonRay r, float3 aabbMin, float3 aabbMax, out float tMaxEnter, out float tMinExit)
{
	float3 invDir = rcp(r.dir);
	float3 tBegin = (aabbMin - r.origin) * invDir;
	float3 tEnd =   (aabbMax - r.origin) * invDir;
	 
	float3 tEnter = min(tBegin, tEnd);
	float3 tExit = max(tBegin, tEnd);
	tMaxEnter = max(0.f, max(tEnter.x, max(tEnter.y, tEnter.z)));
	tMinExit = min(tExit.x, min(tExit.y, tExit.z));
	return tMaxEnter < tMinExit;
}
bool isLeaf(kdNode n)
{
	return n.flags & 1;
}
uint getAxis(kdNode n)
{
	return (n.flags & 6) >> 1;
}

uint getRight(kdNode n)
{
	return (n.flags & 0xFFFFFFF8) >> 3;
}

float rayTriIntersect(float3 rayStartPos, float3 rayDirection, float3 v0, float3 v1, float3 v2)
{
	float3 edge1 = v2 - v0;
	float3 edge0 = v1 - v0;
	float3 normal = cross(edge0, edge1);

	float3 p = cross(rayDirection, edge1);
	float det = dot(edge0, p);

	float3 tv = rayStartPos - v0;

	bool wrongSide = dot(normal, rayDirection) >= 0.f;// || det < .00001f && det > -.00001f;

	float invDet = rcp(det);

	float3 q = cross(tv, edge0);

	float u = dot(tv, p) * invDet;
	float v = dot(rayDirection, q) * invDet;

	float value = dot(edge1, q) * invDet;
	bool inside = u >= 0.f && v >= 0.f && u + v <= 1.f;

	return !wrongSide && inside && value > 0.f ? value : INFINITY;
}


float kdTreeRayTrace(PhotonRay r)
{
	float tMaxEnter, tMinExit;
	if (!rayAABBTest(r, SceneBBMin, SceneBBMax, tMaxEnter, tMinExit))
	{
		return INFINITY;
	}
	
	float tMin = tMaxEnter, tMax = tMaxEnter;
	float tHit = INFINITY;

	//The short stack
	uint top = 0;
	uint numElems = 0;
	StackItem elems[STACKSIZE];
	elems[0] = (StackItem)0;
	elems[1] = (StackItem)0;
	elems[2] = (StackItem)0;

	kdNode node;
	uint nodeIdx;
	kdNode root = KdNodes[0];
	uint rootIdx = 0;
	bool pushDown;
	while (tMax < tMinExit)
	{
		//Check if the stack is empty
		if (numElems == 0)
		{
			node = root;
			nodeIdx = rootIdx;
			tMin = tMax;
			tMax = tMinExit;
			pushDown = true;
		} else
		{
			//pop(stack, node, tMin, tMax, nodeIdx);
			node =    elems[top].n;
			tMin =    elems[top].tMin;
			tMax =    elems[top].tMax;
			nodeIdx = elems[top].idx;
			--numElems;
			top = (top - 1) % STACKSIZE;

			pushDown = false;
		}

		while (!isLeaf(node))
		{
			uint axis = getAxis(node);
			float tSplit = (node.plane - r.origin[axis]) / r.dir[axis];

			uint right = getRight(node);

			uint first = r.origin[axis] < node.plane ? nodeIdx + 1 : right;
			uint second = r.origin[axis] < node.plane ? right : nodeIdx + 1;

			if (tSplit >= tMax || tSplit < 0.f)
			{
				node = KdNodes[first];
				nodeIdx = first;
			} else if(tSplit <= tMin)
			{
				node = KdNodes[second];
				nodeIdx = second;
			} else
			{
				//push(stack, KdNodes[second], tSplit, tMax, second);
				top = (top + 1) % STACKSIZE;
				numElems = min(STACKSIZE, numElems + 1);
				elems[top].n = KdNodes[second];
				elems[top].tMin = tSplit;
				elems[top].tMax = tMax;
				elems[top].idx = second;

				node = KdNodes[first];
				nodeIdx = first;
				tMax = tSplit;
				pushDown = false;
			}

			if (pushDown)
			{
				root = node;
				rootIdx = nodeIdx;
			}
		}

		uint numTris = getRight(node);
		uint triIdx = asuint(node.plane);

		for (uint i = 0; i < numTris; ++i)
		{
			Triangle tri = Triangles[triIdx + i];
			float intersect = rayTriIntersect(r.origin, normalize(r.dir), tri.v0, tri.v1, tri.v2);
			tHit = min(intersect, tHit);

			if (tHit < tMax)
			{
				tMax = tMinExit;
			}
		}
	}


	return tHit;
}

float bruteCachedIntersect(PhotonRay r, uint groupIdx)
{
	float closest = INFINITY;

	uint NumTriangles, stride;
	Triangles.GetDimensions(NumTriangles, stride);

	uint startTri = 0;
	while (startTri < NumTriangles)
	{
		GroupMemoryBarrierWithGroupSync();
		uint numTris = min(NumTriangles - startTri, TRICACHESIZE);

		//Load into cache
		for (uint j = groupIdx; j < numTris; j += GROUPSIZE)
		{
			TriCache[j] = Triangles[startTri + j];
		}
		GroupMemoryBarrierWithGroupSync();
		for (uint i = 0; i < numTris; ++i)
		{
			Triangle tri = TriCache[i];
			float intersect = rayTriIntersect(r.origin, normalize(r.dir.xyz), tri.v0, tri.v1, tri.v2);
			closest = min(intersect, closest);
		}
		startTri = startTri + numTris;
	}
	return closest;
}

float bruteIntersect(PhotonRay r)
{
	float closest = INFINITY;
	
	uint NumTriangles, stride;
	Triangles.GetDimensions(NumTriangles, stride);

	for (uint i = 0; i < NumTriangles; ++i)
	{
		Triangle tri = Triangles[i];
		float intersect = rayTriIntersect(r.origin, normalize(r.dir), tri.v0, tri.v1, tri.v2);
		closest = min(intersect, closest);
	}
	return closest;
}

[numthreads(GROUPSIZE, 1, 1)]
void main(uint3 groupId : SV_GroupId, uint3 dispatchId : SV_DispatchThreadId, uint3 threadId : SV_GroupThreadId, uint groupIdx : SV_GroupIndex)
{
	if (dispatchId.x < NumRays)
	{
		PhotonRay r = Rays[dispatchId.x];

		float closest = length(r.dir);
		r.dir /= closest;

		closest = min(closest, kdTreeRayTrace(r));
		//closest = min(closest, bruteCachedIntersect(r, groupIdx));
		//closest = min(closest, bruteIntersect(r));

		float3 originOffset = r.dir * 150.0f;

		r.dir = r.origin + r.dir * closest;
		//r.origin +=originOffset;

		OutRays.Append(r);
		//Rays[dispatchId.x].dir = r.origin + r.dir * closest;
	}
}