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

VS_OUT main(float3 origin : ORIGIN, uint color : COLOR, float3 end : ENDPOINT, float dynamicity : DYNAMICITY)
{
	VS_OUT output;

	output.begin = origin;
	output.end = end;

	output.begin.xy = toLVSpace(origin.xy);
	output.end.xy = toLVSpace(end.xy);

	output.dir.xyz = normalize(end - origin) * dynamicity;
	output.dir.w = asfloat(color);

	return output;
}