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