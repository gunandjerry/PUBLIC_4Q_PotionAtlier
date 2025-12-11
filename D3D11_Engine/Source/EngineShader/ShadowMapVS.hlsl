#include "../EngineShader/Shared.hlsli"
#ifdef VERTEX_SKINNING
cbuffer MatrixPallete : register(b3)
{
    matrix MatrixPalleteArray[128];
    matrix boneWIT[128];
}
#endif

#include "../EngineShader/VSInput.hlsli"
PS_INPUT main(VS_INPUT input) 
{
    PS_INPUT output = (PS_INPUT)0;
    float4 pos = input.Pos;
    Matrix matWorld;
#ifdef VERTEX_SKINNING
    matWorld =  mul(input.BlendWeights[0], MatrixPalleteArray[input.BlendIndecses[0]]);
    matWorld += mul(input.BlendWeights[1], MatrixPalleteArray[input.BlendIndecses[1]]);
    matWorld += mul(input.BlendWeights[2], MatrixPalleteArray[input.BlendIndecses[2]]);
    matWorld += mul(input.BlendWeights[3], MatrixPalleteArray[input.BlendIndecses[3]]);
    matWorld += mul(input.BlendWeights[4], MatrixPalleteArray[input.BlendIndecses[4]]);
    matWorld += mul(input.BlendWeights[5], MatrixPalleteArray[input.BlendIndecses[5]]);
    matWorld += mul(input.BlendWeights[6], MatrixPalleteArray[input.BlendIndecses[6]]);
    matWorld += mul(input.BlendWeights[7], MatrixPalleteArray[input.BlendIndecses[7]]);
#else
    matWorld = World;
#endif 
    pos = mul(pos, matWorld);
    pos = mul(pos, ShadowViews[0]);
    pos = mul(pos, ShadowProjections[0]);
    output.Pos = pos;
    return output;
}