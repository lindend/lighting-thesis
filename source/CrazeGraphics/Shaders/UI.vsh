#include "globals.incl"

struct VS_INPUT
{
	float2 offset : OFFSET;
	float4 pos : POSITION;
	float4 uv : UV;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float3 uv : TEXCOORD0;
	float4 color : COLOR;
	float4 pos : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.uv = float3(input.uv.xy + input.uv.zw * input.offset, input.uv.x - input.uv.w != 0.f ? 1.f : 0.f);
	output.pos = float4(input.pos.xy + input.pos.zw * input.offset, 0.f, 1.f);
	//output.pos = mul(output.pos, ObjTransform);
	output.pos.xy = output.pos.xy * InvResolution * 2.f - float2(1.f, 1.f);
	output.pos.y = -output.pos.y;
	output.color = input.color;

	return output;
}