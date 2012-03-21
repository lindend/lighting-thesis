#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);
Buffer<uint4> NumRays : register(t1);

struct VS_OUT
{
	float4 position : POSITION;
};

uint touint(float v)
{
	return 0xFF & (uint)(v * 255.f);
}

VS_OUT main(uint idx : SV_VertexID, uint inst : SV_InstanceID)
{
	VS_OUT output;

	uint numRays = NumRays[0].x;
	
	PhotonRay ray = Rays[floor((idx + numRays * inst) / 2)];
	output.position.xyz = idx % 2 == 0 ? ray.origin : ray.dir;
	uint encodedColor = touint(ray.power.r) | touint(ray.power.g) << 8 | touint(ray.power.b) << 16;
	output.position.w = asfloat(encodedColor);

	return output;
}