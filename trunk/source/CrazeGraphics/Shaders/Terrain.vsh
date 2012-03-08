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
	float4 wsPos : POS_WS;
	float4 textureWeights : NORMAL2;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	float4 vertInfo = HeightMap.SampleLevel(Point, input.uv, 0);
	float4 worldPos = mul(float4(input.pos.x, vertInfo.a, input.pos.z, 1.0f), ObjTransform);

	output.wsPos = worldPos;
	output.pos = mul(worldPos, ViewProj);
	output.normal = mul(vertInfo.rgb, NormalTfm);
	output.uv = input.uv;
	output.textureWeights = TextureWeights0.SampleLevel(Point, input.uv, 0);

	return output;
}