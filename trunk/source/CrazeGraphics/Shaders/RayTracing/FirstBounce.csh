#include "globals.incl"
#include "RayTracing/PhotonRay.incl"

AppendStructuredBuffer<PhotonRay> OutRays : register(u0);

cbuffer FrustumInfo : register(b1)
{
	float3 corners[8];
	float3 intensity;
	float lightDynamicity;
};

float3 DepthToPos(float depth, float2 uv)
{
	float4 worldPos = mul(float4(float2(uv.x, 1.f - uv.y) * 2.f - 1.f, depth, 1.0f), LightViewProj);
	return worldPos.xyz / worldPos.w;
}

float2 toPolar(float3 v)
{
	return float2(atan(v.y / v.x), acos(v.z));
}

float3 fromPolar(float theta, float phi)
{
	float sinTheta, cosTheta;
	float sinPhi, cosPhi;
	sincos(theta, sinTheta, cosTheta);
	sincos(phi, sinPhi, cosPhi);
	return float3(cosTheta * sinPhi, sinTheta * sinPhi, cosPhi);
}
float3 fromPolar(float2 thetaPhi)
{
	return fromPolar(thetaPhi.x, thetaPhi.y);
}

float3 clampToHemisphere(float3 v, float3 n)
{
	return dot(v, n) < 0.f ? normalize(v - n * dot(n, v)) : normalize(v);
}

static const uint4 indices[] = 
{
	uint4(0, 1, 2, 3), //back
	uint4(5, 4, 7, 6), //forward
	uint4(1, 0, 4, 5), //top
	uint4(1, 5, 6, 2), //right
	uint4(2, 6, 7, 3), //bottom
	uint4(3, 7, 4, 0) //left
};

float3 calcFrustumNormal(int frustumIndex)
{
	//(V2 - V1) x (V3 - V1)
	uint4 side = indices[frustumIndex];
	float3 v0 = corners[side.x];
	float3 v1 = corners[side.y];
	float3 v2 = corners[side.w];
	return normalize(cross(v1 - v0, v2 - v0));
}

bool rayIntersectsFrustum(float3 rayOrigin, float3 rayDir, out float maxEnter, out float minExit)
{
	maxEnter = -213321456621321122131.0f;
	minExit = 1315677621313213321.0f;
	//bool inside[6];
	bool inside = true;
	for(int i = 0; i < 6; ++i)
	{
		float3 frustumN = calcFrustumNormal(i);
		float frustDotDir = dot(frustumN, rayDir);
		float signDist = dot(frustumN , corners[indices[i].x] - rayOrigin);
		inside = inside && (signDist < 0.f || frustDotDir > 0.f);
		
		if(abs(frustDotDir) > 0.00001f) //if == 0, ray is parallel to plane so they either always intersect or never does
		{
			float intersection = signDist / frustDotDir;
			
			if(frustDotDir > 0.f)
			{
				maxEnter = max(intersection, maxEnter);
			}
			else
			{
				minExit = min(intersection, minExit);
			}
		}
	}
	//return inside;

	return maxEnter < minExit && minExit > 0.f;
}

uint touint(float v)
{
	return 0xFF & (uint)(v * 255.f);
}

#define RAYS_PER_TEXEL 1
[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
	float3 normal = NormalRough[dispatchId.xy].xyz;
	//Check if this sample has a valid normal, ignore it if not.
	if (dot(normal, normal) < 0.5f)
	{
		return;
	}

	float width, height;
	ColorSpec.GetDimensions(width, height);
	width = height = 128.f;
	float2 uv = dispatchId.xy / float2(width, height);

	float3 color = ColorSpec[dispatchId.xy].xyz * intensity;
	float depth = Depth[dispatchId.xy].x;
	float3 position = DepthToPos(depth, uv);

	float2 rndUvBase = position.xz * position.y;// / 10.f;//floor(frac(position.xz * 0.001f) * 128.f) / 128.f;

	for(int i = 0; i < RAYS_PER_TEXEL; ++i)
	{
		//sample direction based on RSM texel world space
		float4 random = Random[(dispatchId.xy + 5 * i + Seed * 1000.f) % 512];
		float3 dir = random.xyz * 2.f - 1.f;

		//Check if the random direction is pointing away from the surface. Reverse it if it doesn't.
		dir = dot(dir, normal) >= 0.f ? dir : -dir;

		//Scale the power based on how many rays for every pixel that are used.
		float power = 1.0f / RAYS_PER_TEXEL;

		float lastIn, firstOut;
		if (rayIntersectsFrustum(position, dir, lastIn, firstOut))
		{
			PhotonRay pr;
			
			pr.dir = normalize(dir);

			float3 rayColor = color * power * dot(pr.dir, normal);
			pr.dir *= firstOut;

			//Encode the color to A8R8G8B8 so it fits into 24 bits
			uint encodedColor = touint(rayColor.r) | (touint(rayColor.g) << 8) | (touint(rayColor.b) << 16);
			pr.color = encodedColor;
			pr.dynamicity = lightDynamicity;
			//pr.dir = dir;
			//Just move out the ray a bit
			pr.origin = position;// + normal * 0.f + max(0.f, lastIn) * pr.dir;
			OutRays.Append(pr);

		}
	}
}