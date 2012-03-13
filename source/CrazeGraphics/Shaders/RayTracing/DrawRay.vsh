#include "globals.incl"
#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);

struct VS_OUT
{
	float3 position : POSITION;
	float3 color : COLOR;
};

VS_OUT main(uint idx : SV_VertexID)
{
	VS_OUT output;

	PhotonRay ray = Rays[idx];
	float3 pos = idx % 2 == 0 ? ray.origin : ray.dir;
	output.position = mul(float4(pos, 1.f), ObjTransform);
	output.color = ray.power;

	return output;
}