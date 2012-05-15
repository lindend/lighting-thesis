#include "globals.incl"
#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);

struct VS_OUT
{
	float3 color : COLOR0;
	float4 begin : POSITION0;
	float4 end : POSITION1;
	float3 dir : DIRECTION;
};

float3 parseColor(uint color)
{
	return float3(	(color & 0xFF) / 255.f,
					((color >> 8) & 0xFF) / 255.f,
					((color >> 16) & 0xFF) / 255.f);
}

VS_OUT main(uint idx : SV_VertexID)
{
	VS_OUT output;

	PhotonRay ray = Rays[idx];
	output.begin = mul(float4(ray.origin, 1.f), ObjTransform);
	output.end = mul(float4(ray.dir, 1.f), ObjTransform);
	output.dir = normalize(ray.dir - ray.origin);
	output.color = parseColor(ray.color);

	return output;
}