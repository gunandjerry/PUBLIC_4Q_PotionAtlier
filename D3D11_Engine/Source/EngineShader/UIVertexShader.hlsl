
#include "Shared.hlsli"

struct UITransform
{
    Matrix World;
	Matrix texcorrdMatrix;
};

struct VSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};



cbuffer UIElement : register(b0)
{
	UITransform transform;
};


PS_INPUT main(VSInput input)
{
	PS_INPUT output = (PS_INPUT) 0;
	output.Pos = mul(input.position, transform.World);
	output.Pos.z = 0.5f;
	output.Pos.w = 1.0f;
	output.Tex = mul(input.texCoord, transform.texcorrdMatrix).xy;
    return output;
}
