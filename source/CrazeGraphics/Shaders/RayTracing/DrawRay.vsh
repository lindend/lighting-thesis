#include "globals.incl"
#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);

struct VS_OUT
{
	float3 color : COLOR0;
	float4 pos : SV_Position;
};

VS_OUT main(uint idx : SV_VertexID)
{
	VS_OUT output;

	PhotonRay ray = Rays[floor(idx / 2)];
	float3 pos = idx % 2 == 0 ? ray.origin : ray.dir;
	output.pos = mul(float4(pos, 1.f), ObjTransform);
	output.color = ray.power;

	return output;
}