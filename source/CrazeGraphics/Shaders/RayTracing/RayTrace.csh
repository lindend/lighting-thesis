#include "RayTracing/PhotonRay.incl"

struct Triangle
{
	float3 v0;
	float3 v1;
	float3 v2;
};

ConsumeStructuredBuffer<PhotonRay> Rays : register(u0);
AppendStructuredBuffer<PhotonRay> OutRays : register(u1);

StructuredBuffer<Triangle> Triangles : register(t0);

#define TRICACHESIZE 512
groupshared Triangle TriCache[TRICACHESIZE];

float rayTriIntersect(float3 rayStartPos, float3 rayDirection, float3 v0, float3 v1, float3 v2)
{
	float INFINITY = 1000000.f;

	float3 edge1 = v2 - v0;
	float3 edge0 = v1 - v0;

	float3 normal = cross(edge0, edge1);
	
	if (dot(normal, rayDirection) >= 0.f)
	{
		return INFINITY;
	}
	
	float3 p = cross(rayDirection, edge1);

	float3 tv = rayStartPos - v0;

	float det = dot(edge0, p);

	if(det < .00001f && det > -.00001f)
	{
		return INFINITY;
	}
	float invDet = 1.f / det;

	float3 q = cross(tv, edge0);

	float u = dot(tv, p) * invDet;
	float v = dot(rayDirection, q) * invDet;


	if(u >= 0.f && v >= 0.f && u + v <= 1.f)
	{
		float value = dot(edge1, q) * invDet;
		if (value > 0.f)
		{
			return value;
		}
		return INFINITY;
	}

	return INFINITY;
}

[numthreads(32, 8, 1)]
void main(uint3 groupId : SV_GroupId, uint3 dispatchId : SV_DispatchThreadId, uint3 threadId : SV_GroupThreadId, uint groupIdx : SV_GroupIndex)
{
	PhotonRay r = Rays.Consume();
	//OutRays.Append(r);
	//return;
	uint NumTriangles, stride;
	Triangles.GetDimensions(NumTriangles, stride);

	float closest = 1000000.f;
	
	for (uint i = 0; i < NumTriangles; ++i)
	{
		Triangle tri = Triangles[i];
		float intersect = rayTriIntersect(r.origin, normalize(r.dir), tri.v0, tri.v1, tri.v2);
		closest = min(intersect, closest);
	} 
	
	if (dot(r.dir, r.dir) > 0.2f)// && closest < 100000.f)
	{
		r.dir = r.origin + r.dir * closest;
		OutRays.Append(r);
	}
}

struct kdTree
{
	boundingBox bbox;
	Buffer<kdNode> nodes;
	uint numTris;
	uint numNodes;
}
struct boundingBox
{
	float3 boxMin;
	float3 boxMax;
}
struct kdNode
{
	//union
	float plane;
	uint triangleIndex;
}

void rayBoxIntersection(float3 rayOrigin, float3 rayDirection, boundingBox bb, out float tMin, out float tMax, out bool intersection)
{
	
}

float kdTreeRayTrace(kdTree tree, float3 rayOrigin, float3 rayDirection)
{
	uint numNodes = tree.numNodes;
	kdNode nodes = tree.nodes;
	uint numTris = tree.numTris;

	
	float bb_tMin;
	float bb_tMax;
	bool intersects;
	rayBoxIntersection(rayOrigin, rayDirection, tree.bbox, bb_tMin, bb_tMax, intersects);
	
	float tMin = bb_tMin;
	float tMax = bb_tMin;

	if (!intersects)
	{
		return -1.f;
	}
	
	float tHit = INFINITY;

	//ShortStack stack = create();

	// kdNode n;
	// float tMin;
	// float tMax;
	// uint idx;

	kdNode node;
	uint nodeIdx;
	kdNode root = tree.nodes[0];
	uint rootIdx = 0;
	bool pushDown;
	while (tMax < tMax)
	{
		if (isEmpty(stack))
		{
			node = root;
			nodeIdx = rootIdx;
			tMin = tMax;
			tMax = rbt.tMax;
			pushDown = true;
		}
		else
		{
			pop(stack, node, tMin, tMax, nodeIdx);
			pushDown = false;
		}

		while (!isLeaf(node))
		{
			uint axis = getAxis(node);
			float tSplit = (node.plane - r.m_Origin.vec.m128_f32[axis]) / r.m_Dir.vec.m128_f32[axis];

			uint right = getRight(node);

			uint first = rayOrigin.vec.m128_f32[axis] < node.plane ? nodeIdx + 1 : right;
			uint second = rayOrigin.vec.m128_f32[axis] < node.plane ? right : nodeIdx + 1;

			if (tSplit >= tMax || tSplit < 0.f)
			{
				node = pNodes[first];
				nodeIdx = first;
			}
			else if(tSplit <= tMin)
			{
				node = pNodes[second];
				nodeIdx = second;
			}
			else
			{
				push(stack, pNodes[second], tSplit, tMax, second);
				node = pNodes[first];
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
		uint triIdx = node.triIndex;

		for (uint i = 0; i < numTris; ++i)
		{
			float3 n;
			float intersect = Intersection::Test(r, pTris[triIdx + i]);
			tHit = Min(intersect, tHit);

			if (tHit < tMax)
			{
				tMax = rbt.tMax;
			}
		}
	}


	return tHit;
}