#include "RayTracing/PhotonRay.incl"
#include "RayTracing/triangleRayIntersection.incl"

struct Triangle
{
	float4 v0;
	float4 v1;
	float4 v2;
};

cbuffer RayTraceInfo : register (b0)
{
	uint NumTriangles;
};

ConsumeStructuredBuffer<PhotonRay> Rays : register(u0);
AppendStructuredBuffer<PhotonRay> OutRays : register(u1);

StructuredBuffer<Triangle> Triangles : register(t0);

#define TRICACHESIZE 512
groupshared Triangle TriCache[TRICACHESIZE];

[numthreads(32, 8, 1)]
void main(uint3 groupId : SV_GroupId, uint3 dispatchId : SV_DispatchThreadId, uint3 threadId : SV_GroupThreadId, uint groupIdx : SV_GroupIndex)
{
	PhotonRay r = Rays.Consume();

	float closest = 10000.f;

	//Iterate so that we can cover all the triangles
	for (int k = 0; k < ceil(NumTriangles / TRICACHESIZE); ++k)
	{
		GroupMemoryBarrierWithGroupSync();
		const int maxIdx = min(TRICACHESIZE, NumTriangles - (k * TRICACHESIZE));
		//Load triangles to cache
		for (int i = groupIdx; i < maxIdx; i += 32 * 8)
		{
			TriCache[i] = Triangles[i + k * TRICACHESIZE];
		}

		GroupMemoryBarrierWithGroupSync();
		
		for (int i = 0; i < maxIdx; ++i)
		{
			Triangle tri = TriCache[i];
			closest = min(intersection(r.origin, r.dir, tri.v0.xyz, tri.v1.xyz, tri.v2.xyz), closest);
		}
	}
	
	r.dir = r.origin + r.dir * closest;
	
	OutRays.Append(r);
}