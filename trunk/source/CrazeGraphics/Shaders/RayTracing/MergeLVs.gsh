struct GS_INPUT
{
	float3 uv : TEXCOORD0;
	float4 pos : SV_POSITION;
};

struct GS_OUTPUT
{
	float3 uv : TEXCOORD;
	float4 pos : SV_Position;
	uint rta : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void main(triangle GS_INPUT input[3], inout TriangleStream<GS_OUTPUT> stream)
{
	for (int i = 0; i < 3; ++i)
	{
		GS_OUTPUT output;
		output.uv = input[i].uv;
		output.pos = input[i].pos;
		output.rta = input[i].uv.z;
		stream.Append(output);
	}
	stream.RestartStrip();
}
