
#include <Shared.hlsli>
Texture2D<float4> gTexture : register(t10);
Texture2D<float4> texturForsize : register(t11);
RWTexture2D<float4> gResult : register(u1);

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 texSize, texSize2;
	texturForsize.GetDimensions(texSize.x, texSize.y);
	gResult.GetDimensions(texSize2.x, texSize2.y);

	float4 color = float4(0, 0, 0, 0);
	float weights[5] = { 0.1201, 0.2339, 0.2931, 0.2339, 0.1201 }; // 5x5 가우시안 커널
	float offsets[5] = { -2, -1, 0, 1, 2 };
	float2 uv = DTid.xy / texSize2;
	float offset = 1.0 / (2.0 * texSize);
	
	for (int i = 0; i < 5; i++)
	{
		float2 samplePos = uv + (float2(0, offsets[i]) + 0.5) / texSize;
		color += gTexture.SampleLevel(ClampSampler, samplePos, 0) * weights[i];
	}

	gResult[DTid.xy] = color;
}
