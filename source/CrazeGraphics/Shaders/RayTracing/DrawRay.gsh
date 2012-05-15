struct GS_INPUT
{
	float3 color : COLOR0;
	float4 begin : POSITION0;
	float4 end : POSITION1;
	float3 dir : DIRECTION;
};

struct GS_OUTPUT
{
	float3 color : COLOR0;
	float uv : TEXCOORD;
	float4 pos : SV_Position;
};

[maxvertexcount(2)]
void main(point GS_INPUT input[1], inout LineStream<GS_OUTPUT> stream)
{
	GS_OUTPUT output0, output1;
	output0.pos = input[0].begin;
	output1.pos = input[0].end;
	output0.color = input[0].color;
	output1.color = input[0].color;
	output0.uv = 0.f;
	output1.uv = 1.f;
	
	stream.Append(output0);
	if (input[0].dir.y < 0.f || true)
	{
		stream.Append(output1);
	} else
	{
		stream.Append(output0);
	}
	stream.RestartStrip();
}
