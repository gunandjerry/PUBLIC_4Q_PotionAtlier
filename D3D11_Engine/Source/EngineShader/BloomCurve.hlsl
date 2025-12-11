#include <PostProcessHeader.hlsli>
#include <Shared.hlsli>

//#define BLOOMCURVE_METHOD_1
//#define BLOOMCURVE_METHOD_2
#define BLOOMCURVE_METHOD_3

RWTexture2D<float4> T_BloomTexture : register(u1);
cbuffer BloomParams : register(b0)
{
	uint bloomCurveMethod;
	float curveThreshold;
	float bloomIntensity;
	float pad;
}

float GetBloomCurve(float x)
{
	float result = x;
	x *= 2.0f;
    
	switch (bloomCurveMethod)
	{
		case 0:
			result = x * 0.05 + max(0, x - curveThreshold) * 0.5; // default threshold = 1.26
			break;
		case 1:
			result = x * x / 3.2;
			break;
		case 2:
			result = max(0, x - curveThreshold); // default threshold = 1.0
			result *= result;
			break;
	}
    
	return result * 0.5f;
}

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint width, height;
	T_Input.GetDimensions(width, height);
	
	
	uint2 outCoord = DTid.xy;
	uint2 inCoord = outCoord * 4;
    
	float4 sum = float4(0, 0, 0, 0);
    [unroll]
	for (uint y = 0; y < 4; y++)
	{
        [unroll]
		for (uint x = 0; x < 4; x++)
		{
			float4 sample = T_Input.Load(int3(inCoord + uint2(x, y), 0));
			sum += sample;
		}
	}
    
	float4 avgColor = sum / 16.0;
	
	float intencity = dot(avgColor, 0.3);
	float bloomIntencity = GetBloomCurve(intencity);
	float3 bloomColor = avgColor * bloomIntencity / intencity;
	T_BloomTexture[DTid.xy] = max(0, float4(bloomColor, 1));

}