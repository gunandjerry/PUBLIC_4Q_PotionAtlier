
#ifndef __Light_HLSL__
#define __Light_HLSL__

#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/BRDF.hlsli"


struct DirectionLight
{
	float4 LightColor;
	float4 LightDir;
	
	Matrix LightViewProj;
};

float3 GetRadiance(DirectionLight light)
{
	return light.LightColor.rgb * light.LightColor.a;
}

float3 GetDirection(DirectionLight light)
{
	return normalize(light.LightDir.xyz);
}

struct PointLight
{
	float4 LightColor;
	float4 LightPosition;
};

float3 GetDirection(PointLight light, float3 position)
{
	return normalize(position - light.LightPosition.xyz);
}

float3 GetRadiance(PointLight light, float3 position)
{
	return max(0, light.LightColor.rgb * light.LightColor.a - length(position - light.LightPosition.xyz));
}

StructuredBuffer<DirectionLight> DirecLights : register(t16);
StructuredBuffer<PointLight> PointLights : register(t17);
Texture2DArray ShadowMap : register(t20);


float3 DefaultLit(float3 albedo, float metallic, float roughness, float3 F0, float3 world, float3 N, float3 V, float3 specular, float ambiantOcclusion)
{
	float2 offsets[9] =
	{
		float2(-1, -1), float2(+0, -1), float2(+1, -1),
		float2(-1, +0), float2(+0, +0), float2(+1, +0),
        float2(-1, +1), float2(+0, +1), float2(+1, +1)
	};
	float3 finalColor = 0;
	uint count, width, height, lod;
	ShadowMap.GetDimensions(0, width, height, count, lod);
	
	float texelSize = 1.0f / width; // 텍셀 크기 (Shadow Map 해상도 기준)

	for (uint i = 0; i < count; i++)
	{
		float shadowFactor = 0.0f;
		
		DirectionLight light = DirecLights[i];
		float4 shadowNDC = mul(float4(world, 1.0f), light.LightViewProj);
		shadowNDC = shadowNDC / shadowNDC.w;
		float currentDepth = shadowNDC.z;
		float2 uv = shadowNDC.xy;
		uv.y = -uv.y;
		uv = uv * 0.5f + 0.5f;
		for (int x = -3; x <= 3; ++x)
		{
			for (int y = -3; y <= 3; ++y)
			{
				float2 offset = float2(x, y) * texelSize;
				float3 sampleUV = float3(uv + offset, i);
				float shadowDepth = ShadowMap.SampleLevel(BorderSampler, sampleUV, 0).r;
				//shadowFactor += 1 - saturate((currentDepth - shadowDepth) * 200.0f);
				shadowFactor += (shadowDepth <= (currentDepth - 0.01)) ? 0.0 : 1.0;
			}
		}
		
		shadowFactor /= 49;
		
		finalColor += BRDF(albedo, metallic, roughness, F0, N, V, GetDirection(DirecLights[i])) * GetRadiance(DirecLights[i]) * shadowFactor * specular;
	}
	uint count2, stride;
	PointLights.GetDimensions(count2, stride);
	for (uint i = 0; i < count2; i++)
	{
		finalColor += BRDF(albedo, metallic, roughness, F0, N, V, GetDirection(PointLights[i], world)) * GetRadiance(PointLights[i], world) * specular;
	}

	finalColor += BRDF_IBL(albedo, metallic, roughness, F0, N, V) * ambiantOcclusion;
	
	return finalColor;
}

float3 DefaultUnLit(float3 albedo, float metallic, float roughness, float3 F0, float3 world, float3 N, float3 V, float3 specular, float ambiantOcclusion)
{
	return 0;
}

float3 StylizedLit(float3 albedo, float metallic, float roughness, float3 F0, float3 world, float3 N, float3 V, float3 specular, float ambiantOcclusion)
{
	float2 offsets[9] =
	{
		float2(-1, -1), float2(+0, -1), float2(+1, -1),
		float2(-1, +0), float2(+0, +0), float2(+1, +0),
        float2(-1, +1), float2(+0, +1), float2(+1, +1)
	};
	float3 finalColor = 0;
	uint count, width, height, lod;
	ShadowMap.GetDimensions(0, width, height, count, lod);
	float texelSize = 1.0f / width; // 텍셀 크기 (Shadow Map 해상도 기준)

	for (uint i = 0; i < count; i++)
	{
		float shadowFactor = 0.0f;
		
		DirectionLight light = DirecLights[i];
		float4 shadowNDC = mul(float4(world, 1.0f), light.LightViewProj);
		shadowNDC = shadowNDC / shadowNDC.w;
		float currentDepth = shadowNDC.z;
		float2 uv = shadowNDC.xy;
		uv.y = -uv.y;
		uv = uv * 0.5f + 0.5f;
		for (int x = -3; x <= 3; ++x)
		{
			for (int y = -3; y <= 3; ++y)
			{
				float2 offset = float2(x, y) * texelSize;
				float3 sampleUV = float3(uv + offset, i);
				float shadowDepth = ShadowMap.SampleLevel(BorderSampler, sampleUV, 0).r;
				//shadowFactor += 1 - saturate((currentDepth - shadowDepth) * 200.0f);
				shadowFactor += (shadowDepth <= (currentDepth - 0.01)) ? 0.0 : 1.0;
			}
		}
		
		shadowFactor /= 49;
		
		finalColor += BRDF_Stylized(albedo, metallic, roughness, F0, N, V, GetDirection(DirecLights[i])) * GetRadiance(DirecLights[i]) * shadowFactor * specular;
	}
	uint count2, stride;
	PointLights.GetDimensions(count2, stride);
	for (uint i = 0; i < count2; i++)
	{
		finalColor += BRDF_Stylized(albedo, metallic, roughness, F0, N, V, GetDirection(PointLights[i], world)) * GetRadiance(PointLights[i], world) * specular;
	}

	
	
	
	finalColor += BRDF_IBL_Stylized(albedo, metallic, roughness, F0, N, V) * ambiantOcclusion;
	
	return finalColor;
}

#define SHADINGMODELID_DEFAULT_LIT 0
#define SHADINGMODELID_DEFAULT_UNLIT 1
#define SHADINGMODELID_DEFAULT_STYLIZED_LIT 2
#define SHADINGMODELID_DEFAULT_UI 3


float3 IntegrateBXDF(float3 albedo, float metallic, float roughness, float3 F0, float3 world, float3 N, float3 V, float3 specular, float ambiantOcclusion, uint shadingModel)
{
	switch (shadingModel)
	{
		case SHADINGMODELID_DEFAULT_LIT:
			return DefaultLit(albedo, metallic, roughness, F0, world, N, V, specular, ambiantOcclusion);
		case SHADINGMODELID_DEFAULT_STYLIZED_LIT:
			return StylizedLit(albedo, metallic, roughness, F0, world, N, V, specular, ambiantOcclusion);
		case SHADINGMODELID_DEFAULT_UNLIT:
		case SHADINGMODELID_DEFAULT_UI:
			return albedo;
	}
	return 0;
}

float IsToneMapping(uint shadingModel)
{
	switch (shadingModel)
	{
		case SHADINGMODELID_DEFAULT_LIT:
			return 1;
		case SHADINGMODELID_DEFAULT_STYLIZED_LIT:
			return 1;
		case SHADINGMODELID_DEFAULT_UNLIT:
		case SHADINGMODELID_DEFAULT_UI:
			return 0;
	}
	return 0;

}


#endif // __Light_HLSL__

