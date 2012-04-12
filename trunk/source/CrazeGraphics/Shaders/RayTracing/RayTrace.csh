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

#define TRICACHESIZE 128
groupshared Triangle TriCache[TRICACHESIZE];

float rayTriIntersect(float3 rayStartPos, float3 rayDirection, float3 v0, float3 v1, float3 v2)
{
	float INFINITY = 1000000.f;

	float3 edge1 = v2 - v0;
	float3 edge0 = v1 - v0;
	float3 normal = cross(edge0, edge1);

	float3 p = cross(rayDirection, edge1);
	float det = dot(edge0, p);

	float3 tv = rayStartPos - v0;

	bool wrongSide = dot(normal, rayDirection) >= 0.f || det < .00001f && det > -.00001f;

	float invDet = rcp(det);

	float3 q = cross(tv, edge0);

	float u = dot(tv, p) * invDet;
	float v = dot(rayDirection, q) * invDet;

	float value = dot(edge1, q) * invDet;
	bool inside = u >= 0.f && v >= 0.f && u + v <= 1.f;

	return !wrongSide && inside && value > 0.f ? value : INFINITY;
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
	
	uint startTri = 0;
	while (startTri < NumTriangles)
	{
		GroupMemoryBarrierWithGroupSync();
		uint numTris = min(NumTriangles - startTri, TRICACHESIZE);

		//Load into cache
		for (uint j = groupIdx; j < numTris; j += 32 * 8)
		{
			TriCache[j] = Triangles[startTri + j];
		}
		GroupMemoryBarrierWithGroupSync();
		for (uint i = 0; i < numTris; ++i)
		{
			Triangle tri = TriCache[i];
			float intersect = rayTriIntersect(r.origin, normalize(r.dir), tri.v0, tri.v1, tri.v2);
			closest = min(intersect, closest);
		}
		startTri = startTri + numTris;
	}
	/*
	for (uint i = 0; i < NumTriangles; ++i)
	{
		Triangle tri = Triangles[i];
		float intersect = rayTriIntersect(r.origin, normalize(r.dir), tri.v0, tri.v1, tri.v2);
		closest = min(intersect, closest);
	} */
	
	if (dot(r.dir, r.dir) > 0.2f)// && closest < 100000.f)
	{
		r.dir = r.origin + r.dir * (closest - 2.f);
		OutRays.Append(r);
	}
}