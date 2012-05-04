#include "globals.incl"
#include "RayTracing/PhotonRay.incl"

StructuredBuffer<PhotonRay> rays : register(t0);
AppendStructuredBuffer<PhotonRay> outRays : register(u0);

[numthreads(128, 1, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
	PhotonRay ray = rays[dispatchId.x];

	uint3 zBufferDimensions;
	Depth.GetDimensions(0, zBufferDimensions.x, zBufferDimensions.y, zBufferDimensions.z);

	float4 clipSpaceBegin = mul(float4(ray.begin, 1.f), ViewProj);
	clipSpaceBegin.xyz /= clipSpaceBegin.w;
	float4 clipSpaceEnd = mul(float(ray.end), 1.f), ViewProj);
	clipSpaceEnd.xyz /= clipSpaceEnd.w;

	float2 clipSpaceMin = min(clipSpaceBegin.xy, clipSpaceEnd.xy) * float2(0.5f, -0.5f) + 0.5f;
	float2 clipSpaceMax = max(clipSpaceBegin.xy, clipSpaceEnd.xy) * float2(0.5f, -0.5f) + 0.5f;

	float2 size = (clipSpaceMax - clipSpaceMin) * zBufferDimensions.xy;
	float maxSize = max(size.x, size.y);
	float mipLevel = ceil(log2(maxSize));
	
	float sample0 = Depth.SampleLevel(Point, clipSpaceMin, mipLevel).x;
	float sample1 = Depth.SampleLevel(Point float2(clipSpaceMin.x, clipSpaceMax.y), mipLevel).x;
	float sample2 = Depth.SampleLevel(Point, float2(clipSpaceMax.x, clipSpaceMin.y), mipLevel).x;
	float sample3 = Depth.SampleLevel(Point, clipSpaceMax, mipLevel).x;

	float maxDepth = max(max(sample0, sample1), max(sample2, sample3));

	if (min(clipSpaceBegin.z, clipSpaceEnd.z) > maxDepth)
	{
		outRays.Append(ray);
	}
}