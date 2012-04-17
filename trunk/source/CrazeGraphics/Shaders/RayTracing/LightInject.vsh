#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);

struct VS_OUT
{
	float4 position : POSITION;
};



VS_OUT main(uint idx : SV_VertexID, uint inst : SV_InstanceID)
{
	VS_OUT output;

	PhotonRay ray = Rays[inst];
	output.position.xyz = idx == 0 ? ray.origin : ray.dir.xyz;
	output.position.w = asfloat(ray.color);

	return output;
}