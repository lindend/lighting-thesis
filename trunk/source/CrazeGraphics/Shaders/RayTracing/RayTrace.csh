#include "RayTracing/PhotonRay.incl"
#include "RayTracing/RTKernel.incl"

#define GROUPSIZE 128
#define TRICACHESIZE 128

StructuredBuffer<PhotonRay> Rays : register(t2);
AppendStructuredBuffer<PhotonRay> OutRays : register(u0);

groupshared Triangle TriCache[TRICACHESIZE];
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

		closest = min(closest, kdTreeRayTrace(r)) + 10.f;
		//closest = min(closest, bruteCachedIntersect(r, groupIdx));
		//closest = min(closest, bruteIntersect(r));

		float3 originOffset = r.dir * 150.0f;

		r.dir = r.origin + r.dir * closest;
		//r.origin +=originOffset;

		OutRays.Append(r);
		//Rays[dispatchId.x].dir = r.origin + r.dir * closest;
	}
}