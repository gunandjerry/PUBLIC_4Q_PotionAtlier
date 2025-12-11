// -----------------------------------------------------
// [FXAA Compute Shader Example]
// Created by : ChatGPT o1
// -----------------------------------------------------

#include <Shared.hlsli>
#include <PostProcessHeader.hlsli>

// ----------------------------------------------------------------
// 컴퓨트 셰이더에서 참조할 상수/함수
// ----------------------------------------------------------------

// (예시) 상수 버퍼. cbuffer 또는 ConstantBuffer<T> 형식.
cbuffer FXAAConstants : register(b0)
{
	uint gScreenWidth;
	uint gScreenHeight;
	float2 gInvScreenSize; // = (1.0/width, 1.0/height)

	float FXAA_SPAN_MAX; // 최대 탐색 거리(픽셀 단위)
	float FXAA_REDUCE_MUL; // 감쇠계수 (보정 강도 조절)
	float FXAA_REDUCE_MIN; // 최소 임계값 (에지가 이 값보다 작으면 무시)
};

float3 LoadColor(float2 uv)
{
	return T_Input.SampleLevel(ClampSampler, uv, 0).rgb;
}

// UAV에 결과 저장
void StoreColor(uint px, uint py, float4 color)
{
	T_Output[uint2(px, py)] = color;
}

[numthreads(64, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID,
             uint3 groupID : SV_GroupID,
             uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // dispatchThreadID.xy -> (픽셀 좌표)
    // dispatchThreadID.z  -> (일반적인 2D 렌더링에서는 0일 것)

    // -------------------------------
    // 1. 화면 범위를 벗어나는지 체크
    // -------------------------------
	if (dispatchThreadID.x >= gScreenWidth ||
        dispatchThreadID.y >= gScreenHeight)
	{
		return;
	}

    // 픽셀 좌표
	uint px = dispatchThreadID.x;
	uint py = dispatchThreadID.y;

    // UV 좌표 (0~1)
	float2 uv = float2((float) px + 0.5f, (float) py + 0.5f) * gInvScreenSize;
    // (0.5f는 텍셀 센터 샘플링을 위한 offset, 선택적)

    // -------------------------------
    // 2. 중심 픽셀 및 주변 픽셀 색상 샘플링
    // -------------------------------
	float3 rgbM = LoadColor(uv);

	float2 texel = float2(gInvScreenSize.x, gInvScreenSize.y);

	float3 rgbN = LoadColor(uv + float2(0.0, -texel.y));
	float3 rgbS = LoadColor(uv + float2(0.0, texel.y));
	float3 rgbW = LoadColor(uv + float2(-texel.x, 0.0));
	float3 rgbE = LoadColor(uv + float2(texel.x, 0.0));

    // -------------------------------
    // 3. 에지 검출 (루미넌스 기반)
    // -------------------------------
	float lumM = dot(rgbM, float3(0.299, 0.587, 0.114));
	float lumN = dot(rgbN, float3(0.299, 0.587, 0.114));
	float lumS = dot(rgbS, float3(0.299, 0.587, 0.114));
	float lumW = dot(rgbW, float3(0.299, 0.587, 0.114));
	float lumE = dot(rgbE, float3(0.299, 0.587, 0.114));

	float lumMax = max(lumM, max(max(lumN, lumS), max(lumW, lumE)));
	float lumMin = min(lumM, min(min(lumN, lumS), min(lumW, lumE)));
	float lumRange = lumMax - lumMin;

    // -------------------------------
    // 4. 에지가 충분히 약하면 그냥 원본 출력
    // -------------------------------
	if (lumRange < FXAA_REDUCE_MIN)
	{
		StoreColor(px, py, float4(rgbM, 1.0));
		return;
	}

    // -------------------------------
    // 5. 에지 방향(dir) 계산
    // -------------------------------
	float2 dir;
    // 수직방향(상+하 - 2*중심) => x
	dir.x = -((lumN + lumS) - 2.0 * lumM);
    // 수평방향(좌+우 - 2*중심) => y
	dir.y = ((lumW + lumE) - 2.0 * lumM);

    // 정규화
	float maxDir = max(abs(dir.x), abs(dir.y));
	if (maxDir > 1e-5)
	{
		dir /= maxDir;
	}

    // -------------------------------
    // 6. 에지 방향으로 추가 샘플링
    // -------------------------------
	float2 offset = dir * FXAA_SPAN_MAX * gInvScreenSize;
    // 예: 최대 8픽셀 거리(기본값 8.0)

	float3 rgb1 = LoadColor(uv + offset * (1.0 / 3.0));
	float3 rgb2 = LoadColor(uv + offset * (2.0 / 3.0));
	float3 fxaaResult = 0.5 * (rgb1 + rgb2);

    // -------------------------------
    // 7. 결과 블렌딩
    // -------------------------------
	float blendFactor = saturate(lumRange * 2.0); // 간단 예시
	float3 finalColor = lerp(rgbM, fxaaResult, blendFactor);

    // -------------------------------
    // 8. 최종 색상 출력 (UAV에 저장)
    // -------------------------------
	StoreColor(px, py, float4(finalColor, 1.0));
}
