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

	uint NumTriangles, stride;
	Triangles.GetDimensions(NumTriangles, stride);

	float closest = 1000000.f;
	
	for (uint i = 0; i < NumTriangles; ++i)
	{
		Triangle tri = Triangles[i];
		float intersect = rayTriIntersect(r.origin, normalize(r.dir), tri.v0, tri.v1, tri.v2);
		closest = min(intersect, closest);
	} 
	
	//Iterate so that we can cover all the triangles
	/*for (int k = 0; k < ceil(NumTriangles / TRICACHESIZE); ++k)
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
			float intersect = rayTriIntersect(r.origin, normalize(r.dir), tri.v0.xyz, tri.v1.xyz, tri.v2.xyz);
			closest = min(intersect, closest);
		}
	}*/
	  
	if (dot(r.dir, r.dir) > 0.2f)// && closest < 100000.f)
	{
		r.dir = r.origin + r.dir * closest;
		OutRays.Append(r);
	}
}