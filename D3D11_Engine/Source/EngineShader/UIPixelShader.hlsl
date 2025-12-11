

#include "Shared.hlsli"

cbuffer UIElement : register(b0)
{
	float4 color;
};

Texture2D uiTexture : register(t0);

// ui»ùÇÃ¸µ?
SamplerState UISampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 result = uiTexture.Sample(UISampler, input.Tex) * color;
    
    if (result.a < 0.01f)
        discard;
    
    return result;
}