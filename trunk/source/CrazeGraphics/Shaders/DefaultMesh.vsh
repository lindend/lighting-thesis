#include "globals.incl"

struct VS_INPUT
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float4 pos : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.f), ObjTransform);
	output.normal = mul(input.normal, (float3x3)NormalTfm);
	output.uv = input.uv;

	//output.worldRay = mul(input.pos, World).xyz - CameraPos.xyz;
	
	return output;
}