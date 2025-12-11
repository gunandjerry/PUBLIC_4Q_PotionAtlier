
#include "../EngineShader/Shared.hlsli"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

struct InstanceData
{
	Matrix world;
	float radiance;
};

cbuffer InstanceBuffer : register(b4)
{
	InstanceData instanceData;
};

cbuffer cbImmutable
{
	static const float4 g_positions[4] =
	{
		float4(-1, 1, 0, 1),
        float4(1, 1, 0, 1),
        float4(-1, -1, 0, 1),
        float4(1, -1, 0, 1),
	};
	
	static const float2 g_texcoords[4] =
	{
		float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1),
	};
};

struct VSOut
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD1;
	float3 Normal : NORMAL0;
	float3 Tangent : NORMAL1;
	float3 BiTangent : NORMAL2;
};

[maxvertexcount(4)]
void main(point VSOut input[1], inout TriangleStream<PS_INPUT> outputStream)
{
	PS_INPUT output = (PS_INPUT)1;
	
	Matrix world = instanceData.world;
	Matrix VP = mul(View, Projection);
	
    
    // Z축 회전 행렬 생성 (4x4)
	//float3x3 rotation = float3x3(float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1));
	float3x3 rotation = 
	float3x3(
		input[0].Tangent, 
		input[0].BiTangent, 
		input[0].Normal
	);
	
	
	for (uint i = 0; i < 4; i++)
	{
		
		float4 position = g_positions[i];
		position = position * instanceData.radiance;
		position.w = 1;
		
		position.xyz = mul(position.xyz, rotation);
		position = position + input[0].position;
		position = mul(position, world);
		
		output.Tex2.x = input[0].texCoord.x;
		
		output.World = position;
		position = mul(position, View);
		output.Pos = mul(position, Projection);
		
		output.Normal = float3(0, 0, 1);
		output.Tangent = float3(1, 0, 0);
		output.BiTangent = float3(0, 1, 0);
		output.Tex = g_texcoords[i];
		outputStream.Append(output);
	}
	
	outputStream.RestartStrip();
}

