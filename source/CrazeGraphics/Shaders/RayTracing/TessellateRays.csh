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
	
	float2 relZ = findRelZ(r.origin.z, r.dir.z);
	float2 clampRelZ = float2(clamp(relZ.x, 0.f, LVCellSize.w), clamp(relZ.y, 0.f, LVCellSize.w));
	int numZ = ceil(abs(clampRelZ.y - clampRelZ.x));

	float2 lvSpace0 = floor(toLVSpaceBase0(r.origin.xy));
	float2 lvSpace1 = floor(toLVSpaceBase0(r.dir.xy));
	
	//lvSpace will contain 0 if the point is inside the LV,
	float2 insideLV = lvSpace0 * lvSpace1;

	r.origin.xy = toLVSpace(r.origin.xy);
	r.dir.xy = toLVSpace(r.dir.xy);

	uint color = r.color;
	if (max(insideLV.x, insideLV.y) <= 0.f)
	{
		[unroll(16)]
		for (int i = 0; i < 16; ++i)
		{
			if (i < numZ)
			{
				r.color = color | (((i + (uint)clampRelZ.x) & 0xFF) << 24);
				OutRays.Append(r);
			}
		}
	}
}