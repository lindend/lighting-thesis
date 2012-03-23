#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> Rays : register(t0);

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

	PhotonRay ray = Rays[inst];
	output.position.xyz = idx == 0 ? ray.origin : ray.dir;
	uint encodedColor = touint(ray.power.r) | touint(ray.power.g) << 8 | touint(ray.power.b) << 16;
	output.position.w = asfloat(encodedColor);

	return output;
}