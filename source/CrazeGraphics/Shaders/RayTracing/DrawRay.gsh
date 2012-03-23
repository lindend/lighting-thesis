struct GS_INPUT
{
	float3 color : COLOR0;
	float4 begin : POSITION0;
	float4 end : POSITION1;
};

struct GS_OUTPUT
{
	float3 color : COLOR0;
	float4 pos : SV_Position;
};

[maxvertexcount(2)]
void main(point GS_INPUT input[1], inout LineStream<GS_OUTPUT> stream)
{
	GS_OUTPUT output0, output1;
	output0.pos = input[0].begin;
	output1.pos = input[0].begin;
	output0.color = input[0].color;
	output1.color = input[0].color;
	
	stream.Append(output0);
	stream.Append(output1);
	stream.RestartStrip();
}
