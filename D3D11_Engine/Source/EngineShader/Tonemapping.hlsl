#include <PostProcessHeader.hlsli>
#include <Shared.hlsli>


cbuffer TonemappingSetting : register(b0)
{
	uint toneMappingIndex;
	float exposure;
	float pad[2];
};

float3 None(Texture2D Texture, float2 texCoord, float autoExposure)
{
	return LinearToGammaSpace(Texture.SampleLevel(ClampSampler, texCoord, 0).xyz);
}

float3 Reinhard(Texture2D Texture, float2 texCoord, float autoExposure)
{
	float3 texColor = Texture.SampleLevel(ClampSampler, texCoord, 0).rgb;
	texColor *= autoExposure;
	texColor = texColor / (1.0 + texColor);
	float3 retColor = pow(max(texColor, 0.0), 1.0 / 2.2);
	return LinearToGammaSpace(retColor);
}

float3 Filmic(Texture2D Texture, float2 texCoord, float autoExposure)
{
	float3 texColor = Texture.SampleLevel(ClampSampler, texCoord, 0).rgb;
	texColor *= autoExposure;
	float3 x = max(texColor - 0.004, 0.0);
	float3 retColor = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
	return retColor;

}

float3 Uncharted2Kernel(float3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Uncharted2(Texture2D Texture, float2 texCoord, float autoExposure)
{
	float W = 11.2;
	float3 texColor = Texture.SampleLevel(ClampSampler, texCoord, 0).rgb;
	texColor *= autoExposure;
    
	float ExposureBias = 2.0;
	float3 curr = Uncharted2Kernel(ExposureBias * texColor);
    
	float3 whiteScale = 1.0 / Uncharted2Kernel(W);
	float3 color = curr * whiteScale;
    
	float3 retColor = pow(max(color, 0.0), 1.0 / 2.2);
	return retColor;

}

float3 ToneMapping(Texture2D Texture, float2 texCoord, uint index, float autoExposure)
{
	switch (index)
	{
		case 0:
			return None(Texture, texCoord, autoExposure);
		case 1:
			return Reinhard(Texture, texCoord, autoExposure);
		case 2:
			return Filmic(Texture, texCoord, autoExposure);
		case 3:
			return Uncharted2(Texture, texCoord, autoExposure);
	}
	return 0;
}

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float width, height;
	T_Input.GetDimensions(width, height);
	
	float maxDim = max(width, height);
	float mipLevel = floor(log2(maxDim));
	
	float3 avgColor = T_Input.SampleLevel(ClampSampler, float2(0.5, 0.5), mipLevel).rgb;
	float isTonemapping = T_GBuffer[1][DTid.xy].r;
	
	float avgLuminance = dot(avgColor, float3(0.2126, 0.7152, 0.0722));
	avgLuminance = max(avgLuminance, 0.0001);
	
	float autoExposure = 0.18 / avgLuminance;

	
	float2 uv = float2(DTid.x / width, DTid.y / height);
	float3 color = ToneMapping(T_Input, uv, toneMappingIndex * isTonemapping, exposure);
	T_Output[DTid.xy] = float4(color, 0);

}
