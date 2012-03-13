#include "globals.incl"

struct VS_OUT
{
	float3 position : POSITION;
	float3 color : COLOR;
};

VS_OUT main(float3 pos : POSITION0, float3 col : COLOR0)
{
	VS_OUT output;

	output.position = mul(float4(pos, 1.f), ObjTransform);
	output.color = col;

	return output;
}