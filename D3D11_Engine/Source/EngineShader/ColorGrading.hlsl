
#include <PostProcessHeader.hlsli>
#include <Shared.hlsli>



Texture3D T_LUT : register(t10);


[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	unorm float3 originColor = T_Input[DTid.xy].rgb;
	T_Output[DTid.xy] = T_LUT.SampleLevel(ClampSampler, originColor, 0);
}