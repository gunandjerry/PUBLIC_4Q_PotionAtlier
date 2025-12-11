#ifndef __BASEPASSPS_HLSL__
#define __BASEPASSPS_HLSL__

#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/Light.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"

/** Shader Flags
 * USE_ALPHA
 * ALPHA_BLEND
 * ALPHA_TEST
 * FORWARD
 */



#ifndef GetGBufferMaterial 
#define GetGBufferMaterial GetDefaultGBufferMaterial
#endif //GetGBufferMaterial


struct PSResult
{
#ifdef FORWARD
	float4 color : SV_TARGET0;
	float4 GBuffer[4] : SV_TARGET1;
	
#else
	float4 GBuffer[8] : SV_TARGET;
#endif

};

#ifdef ALPHA_BLEND
[earlydepthstencil]
#endif
PSResult main(PS_INPUT input)
{
	GBufferMaterial material = GetGBufferMaterial(input);
	float opacity = material.alpha;

#ifdef ALPHA_TEST
    clip( opacity - material.clipAlpha );
#endif
	
#ifdef DITHERING
	//https://wincnt-shim.tistory.com/395
	static const float DitheringPattern[16] =
	{
		0  / 16.0, 8  / 16.0, 2  / 16.0, 10 / 16.0,
		12 / 16.0, 4  / 16.0, 14 / 16.0, 6  / 16.0,
		3  / 16.0, 11 / 16.0, 1  / 16.0, 9  / 16.0,
		15 / 16.0, 7  / 16.0, 13 / 16.0, 5  / 16.0
	};
	uint DitheringIndex = (uint(input.Pos.x) % 4) * 4 + uint(input.Pos.y) % 4;
	float DitheringOut = 1.0f - DitheringPattern[DitheringIndex];
	clip(opacity - DitheringOut);
#endif // DITHERING
	
	PSResult result = (PSResult)1;
	float3 albedo = material.albedo;
	float metallic = material.metallic;
	float3 specular = material.specular;
	float roughness = material.roughness;
	float3 normal = material.normal;
	float3 emissiveColor = material.emissive;
	float ShadingModelID = material.ShadingModelID;
	float ambiantOcclusion = material.ambiantOcclusion;
	
	normal.b = sqrt(1 - max(0.0f, dot(normal.xy, normal.xy)));
	float3x3 TBN = float3x3(input.Tangent, input.BiTangent, input.Normal);
	float3 N = normalize(mul(normalize(normal), TBN));
	
#ifdef FORWARD
	float3 finalColor = 0;
	albedo = GammaToLinearSpace(albedo);
	float3 F0 = lerp(0.04, albedo, metallic);
    float3 V = normalize(MainCamPos - input.World);
	
#ifndef UI

	emissiveColor = GammaToLinearSpace(emissiveColor);
#endif // UI
	
	finalColor = IntegrateBXDF(albedo, metallic, roughness, F0, input.World, N, V, specular, ambiantOcclusion, ShadingModelID);
	finalColor += emissiveColor;
	result.color.rgb = finalColor;
	result.color.a = opacity;
	
	#define GBUFFER_OFSET 0
#else // FORWARD
	
	static int AlbedoSlot = 0;
	static int SpecularSlot = 1;
	static int NormalSlot = 2;
	static int EmissiveSlot = 3;
	
	result.GBuffer[AlbedoSlot].rgb = albedo;
	result.GBuffer[AlbedoSlot].a = metallic;
	
	result.GBuffer[SpecularSlot].rgb = specular;
	result.GBuffer[SpecularSlot].a = ambiantOcclusion;
	
	result.GBuffer[NormalSlot].rgb = N;
	result.GBuffer[NormalSlot].a = roughness;
	
	result.GBuffer[EmissiveSlot].rgb = emissiveColor;
	result.GBuffer[EmissiveSlot].a = ShadingModelID;
	
	#define GBUFFER_OFSET 4
#endif // FORWARD
	
#ifndef PARTICLE
	result.GBuffer[GBUFFER_OFSET + 0] = material.GBuffer[0];
	Matrix viewPos = mul(World, View);
	result.GBuffer[GBUFFER_OFSET + 0].a = (1 - material.GBuffer[0].a) * viewPos._43;
	result.GBuffer[GBUFFER_OFSET + 1] = material.GBuffer[1];
	result.GBuffer[GBUFFER_OFSET + 1].r = IsToneMapping(ShadingModelID);
	result.GBuffer[GBUFFER_OFSET + 2] = material.GBuffer[2];
	result.GBuffer[GBUFFER_OFSET + 3] = material.GBuffer[3];
#endif // PARTICLE
	
	return result;
}

#endif // __BASEPASSPS_HLSL__