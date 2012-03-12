cbuffer LVInfo : register(c0)
{
	float3 LVStart;
	float3 LVEnd;
	float3 LVCellSize;
};

struct VS_OUT
{
	float3 position : POSITION;
	float3 color : COLOR;
}

float3 main(float3 pos : POSITION0, float3 color : COLOR0)
{
	VS_OUT out;
	out.position = pos;
	out.color = color;

	return out;
}

void clampRay(float3 v0, float3 v1, out float3 outv0, out float3 outv1)
{
	intersect(v0, v1, LVStart, LVEnd);
}

float2 intersect(float2 vectorA, float2 vectorB, out float2 intersectionPoint)
{
	
}