
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/VSInput.hlsli"

//어차피 스키닝된메쉬를 빌보드 할일은 없을거같음

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	Matrix W = World;
	Matrix WIT = WorldInverseTranspose;
	Matrix VP = mul(View, Projection);
	float3 normal = normalize(mul(input.Normal, (float3x3) WIT).xyz);
	float3 tangent = normalize(mul(input.Tangent, (float3x3) WIT).xyz);
	
	float4 worldPos = mul(input.Pos, W);
	float3 billboardWorldPosition = mul(worldPos.xyz, (float3x3) IVM).xyz;
	
	output.World = (float3) worldPos;
	output.Pos = mul(float4(billboardWorldPosition, 1.0f), VP);
	output.Normal = normal;
	output.Tangent = tangent;
    
	output.BiTangent = normalize(cross(output.Normal, output.Tangent));
    
	output.Tex = input.Tex;
    
	
	return output;
}

