cbuffer LVInfo : register(c0)
{
	float3 LVStart;
	float3 LVEnd;
	float3 LVCellSize;
};

struct VS_OUT
{
	float3 position : POSITION;
	float3 color : COLOR0;
};

VS_OUT main(float3 pos : POSITION0, float3 color : COLOR0)
{
	VS_OUT output;
	output.position = pos;
	output.color = color;

	return output;
}