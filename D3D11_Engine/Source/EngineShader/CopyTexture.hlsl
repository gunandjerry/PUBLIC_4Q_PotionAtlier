
#include "Shared.hlsli"
Texture2D T_Input : register(t0);

struct VS_OUTPUT2
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};


float4 main(VS_OUTPUT2 input) : SV_TARGET
{
	return T_Input.SampleLevel(DefaultSampler, input.texcoord, 0);
}