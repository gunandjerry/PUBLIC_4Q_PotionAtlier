#pragma once

#include "RendererCore.h"
#include "StructuredBuffer.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include <map>
#include <string>

struct DirectionLightData
{
	float4 Color;
	float4 Directoin;
	Matrix VP;
};

struct PointLightData
{
	float4 Color;
	float4 position;
};


struct DirectionLightBuffer
{
public:
	DirectionLightBuffer(int capacity = 0);

public:
	void SetTextureSize(uint32_t size);
	uint32_t GetTextureSize() const { return textureSize; }


	void PushDirectLight(std::string_view name, const DirectionLightData& light);
	void PopDirectLight(std::string_view name);
	DirectionLightData& GetDirectLight(std::string_view name);
	DirectionLightData& GetDirectLight(size_t index);




	ConstantBuffer& GetLightCamera(size_t index) { return lightCameras[index]; }
	StructuredBuffer GetDirectLightBuffer() const { return directLightBuffer; }
	ID3D11ShaderResourceView* GetShadowMapArray() const { return (ID3D11ShaderResourceView*)shadowMap; }
	ID3D11DepthStencilView* GetShadowMapDS(int index) const { return shadowMap_DSV[index].Get(); }

	uint32_t size() const { return _size; }
	void UpdateBuffer();

	static void ComputeLightMatrix(const BoundingBox& boundBox,							// 바운딩 박스
								   const Vector3& lightDirection,							// 빛의 방향 (정규화된 벡터)
								   _Out_ XMMATRIX& lightViewMatrix,						// 빛 뷰 행렬
								   _Out_ XMMATRIX& lightProjMatrix,						// 빛 직교 투영 행렬)
								   _Out_opt_ BoundingOrientedBox* outProjectBox = nullptr	// 디버깅용 투영박스
	);

private:
	std::map<std::string, int> directLightNames;
	std::vector<DirectionLightData> directLights;
	StructuredBuffer directLightBuffer;


	std::vector<ConstantBuffer> lightCameras;

	Texture shadowMap;
	std::vector<ComPtr<struct ID3D11DepthStencilView>> shadowMap_DSV;

	uint32_t capacity;
	uint32_t _size;
	uint32_t textureSize{1024};

private:
	void CreateBuffer(UINT newCapacity);
	void CreateView(uint32_t newSize);
};


struct PointLightBuffer
{
public:
	PointLightBuffer(int capacity = 0);

public:
	void PushPointLight(std::string_view name, const PointLightData& light);
	void PopPointLight(std::string_view name);
	PointLightData& GetPointLight(std::string_view name);
	PointLightData& GetPointLight(size_t index);

	StructuredBuffer GetPointLightBuffer() const { return pointLightBuffer; }

	uint32_t size() const { return _size; }
	void UpdateBuffer();


private:
	std::map<std::string, int> pointLightNames;
	std::vector<PointLightData> pointLights;
	StructuredBuffer pointLightBuffer;
	uint32_t capacity;
	uint32_t _size;

};