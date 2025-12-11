
#include <PostProcessHeader.hlsli>
#include <Shared.hlsli>


Texture2D<float4> gTexture : register(t10);


cbuffer BloomParams : register(b0)
{
	int bloomCurveMethod;
	float curveThreshold;
	float bloomIntensity;
}
// Compute Shader
[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	
    // 최종 출력
	T_Output[DTid.xy] = T_Input[DTid.xy] + gTexture[DTid.xy] * bloomIntensity;
}
