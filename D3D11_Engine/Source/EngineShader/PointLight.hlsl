

#include "Shared.hlsli"
#include "Light.hlsli"
#include "PostProcessHeader.hlsli"



[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint width, height;
	T_Output.GetDimensions(width, height);
	
	float2 uv = float2(DTid.x / (float) width, DTid.y / (float) height);
	
	float depth = T_Depth.Load(DTid).r;
	if (depth == 1.0)
	{
		return;
	}
}