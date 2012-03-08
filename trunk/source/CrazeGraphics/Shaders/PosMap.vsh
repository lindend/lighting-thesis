#include "globals.incl"

struct VS_IN
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

struct VS_OUT
{
	float3 wsPos : TEXCOORD0;
	float3 normal : NORMAL0;
	float4 pos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output;
	input.uv.y = 1.f - input.uv.y;//float2(input.uv.x, 1.f - input.uv.y);
	output.pos = float4(input.uv * 2.f - 1.f, 0.f, 1.f);
	output.wsPos = input.pos;
	output.normal = input.normal;
	return output;
}