#include "../EngineShader/Shared.hlsli"
#ifdef VERTEX_SKINNING
cbuffer MatrixPallete : register(b3)
{
    matrix MatrixPalleteArray[320];
    matrix boneWIT[320];
}
#endif

#include "../EngineShader/VSInput.hlsli"
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
	Matrix matWorld = World;
	Matrix WIT = WorldInverseTranspose;
	Matrix VP = mul(View, Projection);
	float3 normal;
	float3 tangent;
    
#ifdef VERTEX_SKINNING
    matWorld = mul(input.BlendWeights[0], MatrixPalleteArray[input.BlendIndecses[0]]);
    [unroll]
    for (int w = 1; w < MAX_WEIGHTS; w++)
    {
        matWorld += mul(input.BlendWeights[w], MatrixPalleteArray[input.BlendIndecses[w]]);
    }
    WIT =  mul(input.BlendWeights[0], boneWIT[input.BlendIndecses[0]]);
    [unroll]
    for (w = 1; w < MAX_WEIGHTS; w++)
    {
        WIT += mul(input.BlendWeights[w], boneWIT[input.BlendIndecses[w]]);
    }
#endif  
    normal = normalize(mul(input.Normal, (float3x3) WIT).xyz);
	tangent = normalize(mul(input.Tangent, (float3x3) WIT).xyz);
      
	float4 worldPos = mul(input.Pos, matWorld);
	output.World = (float3) worldPos;
	output.Pos = mul(worldPos, VP);
	output.Normal = normal;
	output.Tangent = tangent;
    
    output.BiTangent = normalize(cross(output.Normal, output.Tangent));
    
    output.Tex = input.Tex;
    
    return output;
}