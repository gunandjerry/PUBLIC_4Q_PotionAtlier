#define MAX_WEIGHTS 12
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float2 Tex : TEXCOORD0;
   
#ifdef VERTEX_SKINNING
    int BlendIndecses[MAX_WEIGHTS]: BLENDINDICES;
    float BlendWeights[MAX_WEIGHTS] : BLENDWEIGHT;
#endif
};
