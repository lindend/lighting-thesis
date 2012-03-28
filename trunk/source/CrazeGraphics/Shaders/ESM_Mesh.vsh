#include "globals.incl"

struct VS_INPUT
{
	float4 pos : POSITION0;
};

struct VS_OUTPUT
{
	float2 depth : NORMAL0;
	float4 pos : SV_Position;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float4 wsPos = mul(input.pos, ObjTransform);
	output.pos = mul(wsPos, LightViewProj);
	output.depth = output.pos.zw;//float2(0.f, distance(LightPos, wsPos.xyz));

	return output;
}