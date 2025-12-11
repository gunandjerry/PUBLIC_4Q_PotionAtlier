
#include "Shared.hlsli"

struct Particle
{
	float3 position;
	float lifeTime;
	
	float3 velocity;
	float elapsedTime;
	
	float3 acceleration;
	float pad;
	
	float3x3 rotation;
};

StructuredBuffer<Particle> positions : register(t0);

struct VSOut
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD1;
	float3 Normal : NORMAL0;
	float3 Tangent : NORMAL1;
	float3 BiTangent : NORMAL2;
};

VSOut main(uint id : SV_VERTEXID)
{
	VSOut output = (VSOut) 0;
	output.position.xyz = positions[id].position;
	output.texCoord.x = positions[id].elapsedTime;
	
	
	float3x3 temp = positions[id].rotation;
	
	output.Tangent = temp._11_12_13;
	output.BiTangent = temp._21_22_23;
	output.Normal = temp._31_32_33;
	
	return output;
}
