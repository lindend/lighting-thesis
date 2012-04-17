#include "globals.incl"

struct INPUT
{
	float4 dir : DIRECTION;
	float3 begin : POSITION0;
	float3 end : POSITION1;
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
void main(point INPUT input[1], inout LineStream<OUTPUT> output)
{
	OUTPUT output0;
	OUTPUT output1;
	
	output0.dir = output1.dir = input[0].dir;
	uint rtIdx = (asuint(input[0].dir.w) >> 24) & 0xFF;
	output0.rtIdx = rtIdx;
	output1.rtIdx = rtIdx;

	float zPos0 = calcZPos(input[0].begin.z, output0.rtIdx);
	float zPos1 = calcZPos(input[0].end.z, output1.rtIdx);

	output0.pos = float4(input[0].begin.xy, zPos0, 1.f);
	output1.pos = float4(input[0].end.xy, zPos1, 1.f);

	//Append the points in opposite order for rasterization that is better suited to our purposes (does not matter if using antialiasing though)
	output.Append(output1);
	output.Append(output0);
	output.RestartStrip();
}