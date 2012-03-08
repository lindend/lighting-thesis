#include "Light.incl"

cbuffer LightingInfo : register(b0)
{
	row_major float4x4 InvProj :	packoffset(c0);
	uint NumLights :				packoffset(c4);
	float2 InvResolution :			packoffset(c4.y);
	float2 ProjAB :					packoffset(c5);
};

RWTexture2D<float3> Target : register(u0);

Texture2D ColorSpec :				register(t0);
Texture2D NormalRough :				register(t1);
Texture2D Depth :					register(t3);
StructuredBuffer<Light> Lights :	register(t4);

float3 LightPixel(Light l, float4 color, float3 normal, float3 pos);
float3 DepthToPos(float depth, float2 uv);

#define MAXLIGHTS 1024

groupshared uint CachedLights;
groupshared uint LightCache[MAXLIGHTS];

groupshared uint MinTileDepth;
groupshared uint MaxTileDepth;

bool IsVisible(Light l, float minDepth, float maxDepth)
{
	//return true;
	return l.type == DIRECTIONALLIGHT || (l.pos.z + l.pos.w) > minDepth && (l.pos.z - l.pos.w) < maxDepth;
}

#define BLOCKSIZE 16
[numthreads(BLOCKSIZE, BLOCKSIZE, 1)]
void main(uint3 groupId : SV_GroupId, uint3 dispatchId : SV_DispatchThreadId, uint3 threadId : SV_GroupThreadId, uint groupIdx : SV_GroupIndex)
{

	MinTileDepth = 0xFFFFFFFF;
	MaxTileDepth = 0;
	CachedLights = 0;

	GroupMemoryBarrierWithGroupSync();

	float depth = Depth.Load(dispatchId).x;
	InterlockedMin(MinTileDepth, asuint(depth));
	InterlockedMax(MaxTileDepth, asuint(depth));

	GroupMemoryBarrierWithGroupSync();

	float minDepth = ProjAB.y / (asfloat(MinTileDepth) - ProjAB.x);
	float maxDepth = ProjAB.y / (asfloat(MaxTileDepth) - ProjAB.x);

	for (uint i = groupIdx; i < NumLights; i += BLOCKSIZE * BLOCKSIZE)
	{
		Light l = Lights[i];
		if (IsVisible(l, minDepth, maxDepth))
		{
			uint lightIdx;
			InterlockedAdd(CachedLights, 1, lightIdx);
			LightCache[lightIdx] = i;
		}
	}

	GroupMemoryBarrierWithGroupSync();

	float4 color = ColorSpec.Load(dispatchId);
	float4 normRough = NormalRough.Load(dispatchId);
	float3 normal = normRough.xyz;
	float roughness = normRough.w;
	float2 uv = dispatchId.xy * InvResolution;

	float3 result = 0.f;
	
	float3 pos = DepthToPos(depth, uv);

	//A bug is forcing me to do this
	const uint iters = CachedLights;
	for (uint j = 0; j < iters; ++j)
	{
		Light l = Lights[LightCache[j]];
		result += LightPixel(l, color, normal.xyz, pos);
	}

	Target[dispatchId.xy] = result;
}

float3 DepthToPos(float depth, float2 uv)
{
	float4 worldPos = mul(float4(float2(uv.x, 1.f - uv.y) * 2.f - 1.f, depth, 1.0f), InvProj);
	return worldPos.xyz / worldPos.w;
}

float3 LightPixel(Light l, float4 color, float3 normal, float3 pos)
{
	//return color.xyz;
	float fallOff = 0.f;
	float shading = 0.f;
	if (l.type == POINTLIGHT)
	{
		const float3 delta = l.pos.xyz - pos;
		fallOff = 1.f - smoothstep(0, l.pos.w, length(delta));
		shading = saturate(dot(normal, normalize(delta)));
	}
	
	return l.color.xyz * color.xyz * fallOff * shading;
}
