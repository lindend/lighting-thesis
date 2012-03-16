#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);

struct VS_OUT
{
	float3 color : COLOR0;
	float3 position : POSITION;
};

VS_OUT main(uint idx : SV_VertexID, uint inst : SV_InstanceID)
{
	VS_OUT output;

	uint numRays = 5856;
	
	PhotonRay ray = Rays[floor((idx + numRays * inst) / 2)];
	output.position = idx % 2 == 0 ? ray.origin : ray.dir;
	output.color = ray.power;

	return output;
}