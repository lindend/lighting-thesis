#include "RayTracing/PhotonRay.incl"
#include "globals.incl"

RWStructuredBuffer<PhotonRay> Rays : register(u0);
AppendStructuredBuffer<PhotonRay> OutRays : register(u1);



float2 toLVSpaceBase0(float2 pos)
{
	return ((pos - LVStart.xy) / (LVCellSize.xy * LVCellSize.w));
}

float2 toLVSpace(float2 pos)
{
	return ((pos - LVStart.xy) / (LVCellSize.xy * LVCellSize.w)) * 2.f - 1.f;
}

float2 findRelZ(float pos0z, float pos1z)
{
	return (float2(pos0z, pos1z) - LVStart.z) / LVCellSize.z;
}

//Puts the smallest of relZ in x component, and converts both to ints
int2 processRelZ(float2 relZ)
{
	return int2(floor(min(relZ.x, relZ.y)), ceil(max(relZ.x, relZ.y)));
}

float3 parseColor(float v)
{
	uint color = asuint(v);
	return float3(	(color & 0xFF) / 255.f,
					((color >> 8) & 0xFF) / 255.f,
					((color >> 16) & 0xFF) / 255.f);
}

[numthreads(256, 1, 1)]
void main(uint3 dispatchId : SV_DispatchThreadId)
{
	uint numRays, stride;
	Rays.GetDimensions(numRays, stride);
	if (dispatchId.x >= numRays)
	{
		return;
	}

	PhotonRay r = Rays[dispatchId.x];

	float3 rayDelta = r.dir - r.origin;

	float3 intersectStartDist = (LVStart - r.origin) / rayDelta;
	float3 intersectEndDist = (LVEnd - r.origin) / rayDelta;
	float3 minIntersect = max(0.f, min(intersectStartDist, intersectEndDist));
	float3 maxIntersect = min(1.f, max(intersectStartDist, intersectEndDist));

	float tMin = min(min(minIntersect.x, minIntersect.y), minIntersect.z);
	float tMax = max(max(maxIntersect.x, maxIntersect.y), maxIntersect.z);

	//The distance the ray needs to travel to reach a new slice
	float zCellDist = LVCellSize.z / abs(rayDelta);
	float nextZSlice = abs(intersectStartDist.z % zCellDist);

	uint color = r.color;
	float t = tMin;
	uint firstIndex = 0;
	//[unroll(32)]
	for (int i = 0; i < LVCellSize.w; ++i)
	{
		if (t < tMax)
		{
			float tNext = min(t + nextZSlice, tMax);
			nextZSlice = zCellDist;

			PhotonRay tr;
			tr.origin = r.origin;
			tr.dir = r.dir;

			tr.origin = r.origin + rayDelta * t;
			tr.dir = r.origin + rayDelta * tNext;
			tr.dynamicity = r.dynamicity;
			
			//Find the index by calculating the average position of the line segment,
			//and use that position to determine z-index
			float averageZ = (tr.origin.z + tr.dir.z) * 0.5f;
			uint idx = (uint)floor((averageZ - LVStart.z) / LVCellSize.z);
			tr.color = color | ((idx & 0xFF) << 24);
			OutRays.Append(tr);

			t = tNext;
		}
	}
}
