#include "RayTracing/PhotonRay.incl"
#include "globals.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);

struct VS_OUT
{
	float4 dir : DIRECTION;
	float3 begin : POSITION0;
	float3 end : POSITION1;
};

float2 toLVSpace(float2 pos)
{
	return ((pos - LVStart.xy) / (LVCellSize.xy * LVCellSize.w)) * 2.f - 1.f;
}

VS_OUT main(uint idx : SV_VertexID)
{
	VS_OUT output;

	PhotonRay ray = Rays[idx];
	//ray.origin = (float)idx / 10.f;
	//ray.dir = (float)idx / 10.f + 1000.f;
	//
	//uint rtaIdx = idx % 16;
	//ray.color = rtaIdx << 24;
	//
	//ray.dynamicity = 0.02f;

	output.begin = ray.origin;
	output.end = ray.dir;

	output.begin.xy = toLVSpace(ray.origin.xy);
	output.end.xy = toLVSpace(ray.dir.xy);

	output.dir.xyz = normalize(ray.dir - ray.origin) * ray.dynamicity;
	output.dir.w = asfloat(ray.color);

	return output;
}