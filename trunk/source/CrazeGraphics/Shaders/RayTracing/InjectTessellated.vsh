#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);

struct VS_OUT
{
	float4 dir : DIRECTION;
	float3 begin : POSITION0;
	float3 end : POSITION1;
};


VS_OUT main(uint idx : SV_VertexID)
{
	VS_OUT output;

	PhotonRay ray = Rays[idx];
	output.begin = ray.origin;
	output.end = ray.dir;
	output.dir.xyz = normalize(ray.dir - ray.origin);
	output.dir.w = asfloat(ray.color);

	return output;
}