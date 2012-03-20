cbuffer LVInfo : register(c0)
{
	float3 LVStart;
	float3 LVEnd;
	float3 LVCellSize;
};

struct DS_OUTPUT
{
	float4 dir : DIRECTION;
	float4 pos : SV_Position;
};

struct OUTPUT
{
	//float3 color : COLOR;
	float4 dir : DIRECTION;
	float4 pos : SV_Position;
	uint rtIdx : SV_RenderTargetArrayIndex;
};

float calcZPos(float z, int idx)
{
	float startZ = LVStart.z + LVCellSize.z * idx;
	return (z - startZ) / LVCellSize.z;
}

float3 parseColor(float v)
{
	uint color = asuint(v);
	return float3(	(color & 0xFF) / 255.f,
					((color >> 8) & 0xFF) / 255.f,
					((color >> 16) & 0xFF) / 255.f);
}

[maxvertexcount(2)]
void main(line DS_OUTPUT input[2], inout LineStream<OUTPUT> output)
{
	OUTPUT output0;
	OUTPUT output1;
	
	output0.dir = output1.dir = input[0].dir;
	//output0.color = output1.color = parseColor(input[0].dir.w);
	output0.rtIdx = input[0].pos.w;
	output1.rtIdx = input[0].pos.w;

	float zPos0 = calcZPos(input[0].pos.z, output0.rtIdx);
	float zPos1 = calcZPos(input[1].pos.z, output1.rtIdx);

	output0.pos = float4(input[0].pos.xy, zPos0, 1.f);
	output1.pos = float4(input[1].pos.xy, zPos1, 1.f);

	//Append the points in opposite order for rasterization that is better suited to our purposes (does not matter if using antialiasing though)
	output.Append(output1);
	output.Append(output0);
	output.RestartStrip();
}