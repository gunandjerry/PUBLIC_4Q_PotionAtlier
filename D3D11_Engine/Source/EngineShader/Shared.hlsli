#ifndef __SHARED_HLSL__
#define __SHARED_HLSL__

static const float PI = 3.141592654;
static const float3 Fdielectric = 0.04f;
static const float Epsilon = 1e-6;
static const float MinRoughness = 0.04f;

SamplerState DefaultSampler : register(s0);
SamplerState ClampSampler : register(s1);
SamplerState BorderSampler : register(s2);
SamplerState PointSampler : register(s3);


#define SHADOW_MAP_SIZE 8192

inline float GammaToLinearSpaceExact(float value)
{
    if (value <= 0.04045f)
        return value / 12.92f;
    else if (value < 1.0f)
        return pow((value + 0.055f) / 1.055f, 2.4f);
    else
        return pow(value, 2.2f);
}

inline float3 GammaToLinearSpaceExact(float3 value)
{
    return float3(
    GammaToLinearSpaceExact(value.r),
    GammaToLinearSpaceExact(value.g),
    GammaToLinearSpaceExact(value.b));
}

inline float3 GammaToLinearSpace(float3 rgb)
{
    return rgb * (rgb * (rgb * 0.305306011f + 0.682171111f) + 0.012522878f);

    //Precise version, useful for debugging.
    //return float3(GammaToLinearSpaceExact(rgb.r),
    //GammaToLinearSpaceExact(rgb.g),
    //GammaToLinearSpaceExact(rgb.b));
}

inline float LinearToGammaSpaceExact(float value)
{
    if (value <= 0.0f)
        return 0.0f;
    else if (value <= 0.0031308f)
        return 12.92f * value;
    else if (value < 1.0f)
        return 1.055f * pow(value, 0.4166667f) - 0.055f;
    else
        return pow(value, 0.45454545f);
}

inline float3 LinearToGammaSpaceExact(float3 value)
{
    return float3(
    LinearToGammaSpaceExact(value.r),
    LinearToGammaSpaceExact(value.g),
    LinearToGammaSpaceExact(value.b));
}

inline float3 LinearToGammaSpace(float3 rgb)
{
    rgb = max(rgb, float3(0.f, 0.f, 0.f));
    return max(1.055f * pow(rgb, 0.416666667f) - 0.055f, 0.f);

    //Precise version, useful for debugging.
    //return float3(LinearToGammaSpaceExact(rgb.r),
    //LinearToGammaSpaceExact(rgb.g),
    //LinearToGammaSpaceExact(rgb.b));
}

inline float NormalDistribution(float roughness, float NoH)
{
    roughness = max(roughness, MinRoughness);
    float a = roughness * roughness;
    float squareA = a * a;
    float var = NoH * NoH * (squareA - 1.0) + 1.0;
    return squareA / (PI * var * var);
}

inline float3 FresnelReflection(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

inline float GSchlickGGX(float NoX, float k)
{
    return NoX / (NoX * (1.0 - k) + k);
}

inline float GeometricAttenuation(float NoV, float NoL, float roughness)
{
    roughness = max(roughness, MinRoughness);
    float a = roughness + 1.0;
    float k = (a * a) / 8.0; // direct
    //float k = (roughness * roughness) * 0.5; // IBL
    
	return GSchlickGGX(NoV, k);
    return GSchlickGGX(NoV, k) * GSchlickGGX(NoL, k);
}

inline float3 SpecularBRDF(float D, float3 F, float G, float NoL, float NoV)
{
    return D * F * G / max(4.0f * NoL * NoV, 0.001f);
}

inline float3 SpecularIBL(float3 F0, float2 specularBRDF, float3 PrefilteredColor)
{
    return (F0 * specularBRDF.x + specularBRDF.y) * PrefilteredColor;
}

inline float3 DiffuseBRDF(float3 BaseColor, float3 F, float NoL, float Metalness)
{
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), Metalness);
    return float3(BaseColor * kd * NoL / PI);
}

float2 FlipBook(float time, int columns, int rows, float2 texCoord)
{
    // 총 프레임 수
	int totalFrames = columns * rows;
    
    // 프레임 간 간격 (애니메이션이 1초에 한 사이클)
	float frameInterval = 1.0 / totalFrames;
    
    // 현재 프레임 인덱스 계산 및 래핑
	int currentFrame = (int) (time / frameInterval) % totalFrames;
    
    // 열과 행 계산
	int column = currentFrame % columns;
	int row = currentFrame / rows;
    
    // 텍스처의 각 셀 크기
	float2 cellSize = float2(1.0 / columns, 1.0 / rows);
    
	return (texCoord + float2(column, row)) * cellSize;
}

// Texture2D와 SamplerState를 인자로 받아 FlipBook 애니메이션을 보간하여 샘플링합니다.
// time: 0~1 사이의 값 (1초에 한 사이클)
// columns, rows: 스프라이트 시트의 열과 행 수
// texCoord: 각 셀 내부의 로컬 텍스처 좌표 (0~1)
float4 FlipBook(Texture2D tex, float time, int columns, int rows, float2 texCoord)
{
    int totalFrames = columns * rows;

    float framePos = frac(time) * (float) totalFrames;

    int currentFrame = (int) floor(framePos);
    int nextFrame = (currentFrame + 1) % totalFrames; // 마지막 프레임이면 처음으로 순환

    float blendFactor = frac(framePos); //현재 프레임과 다음 프레임 사이의 보간 계수 (0~1)

    //각 셀의 크기 (UV 공간에서)
    float2 cellSize = float2(1.0 / columns, 1.0 / rows);

    //현재 프레임의 UV 좌표 (UV 원래대로 유지)
    int currentCol = currentFrame % columns;
    int currentRow = currentFrame / columns;
    float2 currentUV = (texCoord + float2(currentCol, currentRow)) * cellSize;

    //다음 프레임의 UV 좌표
    int nextCol = nextFrame % columns;
    int nextRow = nextFrame / columns;
    float2 nextUV = (texCoord + float2(nextCol, nextRow)) * cellSize;

    //각 프레임을 텍스처에서 샘플링
    float4 colorCurrent = tex.Sample(DefaultSampler, currentUV);
    float4 colorNext = tex.Sample(DefaultSampler, nextUV);

    //부드럽게 보간하여 최종 색상 계산
    return lerp(colorCurrent, colorNext, blendFactor);
}

// https://www.shadertoy.com/view/4dKcWK

float3 HUEtoRGB(in float hue)
{
    // Hue [0..1] to RGB [0..1]
    // See http://www.chilliant.com/rgb2hsv.html
	float3 rgb = abs(hue * 6. - float3(3, 2, 4)) * float3(1, -1, -1) + float3(-1, 2, 2);
	return clamp(rgb, 0., 1.);
}

float3 HSVtoRGB(in float3 hsv)
{
    // Hue-Saturation-Value [0..1] to RGB [0..1]
	float3 rgb = HUEtoRGB(hsv.x);
	return ((rgb - 1.) * hsv.y + 1.) * hsv.z;
}

float3 RGBtoHCV(in float3 rgb)
{
    // RGB [0..1] to Hue-Chroma-Value [0..1]
    // Based on work by Sam Hocevar and Emil Persson
	float4 p = (rgb.g < rgb.b) ? float4(rgb.bg, -1., 2. / 3.) : float4(rgb.gb, 0., -1. / 3.);
	float4 q = (rgb.r < p.x) ? float4(p.xyw, rgb.r) : float4(rgb.r, p.yzx);
	float c = q.x - min(q.w, q.y);
	float h = abs((q.w - q.y) / (6. * c + Epsilon) + q.z);
	return float3(h, c, q.x);
}

float3 RGBtoHSV(in float3 rgb)
{
    // RGB [0..1] to Hue-Saturation-Value [0..1]
	float3 hcv = RGBtoHCV(rgb);
	float s = hcv.y / (hcv.z + Epsilon);
	return float3(hcv.x, s, hcv.z);
}

cbuffer cbuffer_Transform : register(b0)
{
    Matrix World;
    Matrix WorldInverseTranspose;
    //Matrix WVP;
}

cbuffer cb_Camera : register(b1)
{
    float3 MainCamPos;
    Matrix View;
    Matrix Projection;
    Matrix IVM;
    Matrix IPM;
};

#define MAX_LIGHT_COUNT 4
cbuffer cb_ShadowMap : register(b2)
{
    Matrix ShadowProjections[MAX_LIGHT_COUNT];
    Matrix ShadowViews[MAX_LIGHT_COUNT];
};


struct PerFrameConstants
{
	float Time;
	float Time0_1;
	float deltaTime;
};

cbuffer PerFrame : register(b3)
{
	PerFrameConstants frameData;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 World : POSITION;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float3 BiTangent : NORMAL2;
    float2 Tex : TEXCOORD0;
	float2 Tex2 : TEXCOORD1; // 파티클의 생존시간 넣을    예정
    //float4 PositionShadows[MAX_LIGHT_COUNT] : TEXCOORD1;
};

#endif // __SHARED_HLSL__