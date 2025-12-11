#include "../EngineShader/Shared.hlsli"

Texture2D<float4> gTexture1 : register(t10);
RWTexture2D<float4> gResult : register(u1);

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 texSize;
	gResult.GetDimensions(texSize.x, texSize.y);
	float2 uv = DTid.xy / texSize;
	float offset = 1.0 / (2.0 * texSize);

	gResult[DTid.xy] += max(0, gTexture1.SampleLevel(ClampSampler, uv, 0));
}