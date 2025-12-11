
#include <Shared.hlsli>
Texture2D<float4> g_Input : register(t10);
RWTexture2D<float4> g_Output : register(u1);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint width, height;
	g_Output.GetDimensions(width, height);
	if(DTid.x >= width || DTid.y >= height)
	{
		return;
	}
	uint2 outCoord = DTid.xy;
	uint2 inCoord = outCoord * 6;
    
	float4 sum = float4(0, 0, 0, 0);
    
    [unroll]
	for (uint y = 0; y < 6; y++)
	{
        [unroll]
		for (uint x = 0; x < 6; x++)
		{
			float4 sample = g_Input[inCoord + uint2(x, y)];
			sum += sample;
		}
	}
    
	float4 avgColor = sum / 36.0;
	g_Output[outCoord] = avgColor;
}
