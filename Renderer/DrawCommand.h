#pragma once

#include "RendererCore.h"
#include "ShaderResource.h"
#include "ConstantBuffer.h"
#include "MeshBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "RendererBuffer.h"
#include "SamplerState.h"
#include "StructuredBuffer.h"
#include <Directxtk/SimpleMath.h>
#include <vector>

struct MeshData
{
	RendererBuffer vertexBuffer;
	RendererBuffer indexBuffer;
	uint32_t indexCounts;
	uint32_t vertexStride;
	VertexShader vertexShader;
	std::vector<Binadble> shaderResources;

	DirectX::BoundingOrientedBox boundingBox;
};

struct MaterialData
{
	PixelShader pixelShader;
	std::vector<Binadble> shaderResources;
};


struct MeshDrawCommand
{
public:
	MeshDrawCommand();
	~MeshDrawCommand();

public:
	MeshData meshData;
	MaterialData materialData;
};

namespace EDebugMeshDraw
{
	enum Type
	{
		Box,
		Sphere,
		Ray,
		Frustom
	};
};

struct DebugMeshDrawCommand
{
	DebugMeshDrawCommand();
	~DebugMeshDrawCommand();

	EDebugMeshDraw::Type type;
	union
	{
		DirectX::BoundingOrientedBox boundingBox;
		DirectX::BoundingSphere boundingSphere;
		DirectX::SimpleMath::Ray ray;
		DirectX::BoundingFrustum frustom;
	};
	Vector4 color;

};

struct SkyBoxMeshDrawCommand : public MeshDrawCommand
{
};

struct DispatchData
{
	int x{ 1920 };
	int y{ 1080 };
	int z{ 1 };
};

struct PostProcesCommand
{
	std::vector<ComputeShader> computeShader;
	std::vector<std::function<void()>> computeShaderSet;
	std::vector<Binadble> shaderResources;
	std::vector<DispatchData> dispatchDatas;
	bool isMipMap{ false };
};

// VP 없이 바로 NDC사용
// z는 0.5고정
struct UIDrawCommand
{
	Texture texture;
	ConstantBuffer transformBuffer;
	ConstantBuffer colorBuffer;
	float drawSpeed;
};

/**
 * @brief 머티리얼 기반 UI
 */
struct UIMaterialDrawCommand
{
	ConstantBuffer transformBuffer;
	std::vector<Binadble> shaderResources;
	PixelShader pixelShader;
	float drawSpeed;
};


struct UIMatrixHelper
{
	static Matrix MakeUIMatrix(size_t positionX,
							   size_t positionY,
							   size_t width,
							   size_t height,
							   size_t screenWidth,
							   size_t screenHeight,
							   float rotation_degree = 0
							   );
	static Matrix MakeUIMatrixYFlip(
		size_t positionX,
		size_t positionY,
		long long width,
		long long height,
		size_t screenWidth,
		size_t screenHeight,
		float rotation_degree = 0
	);

};

struct ParticleDrawCommand
{
	StructuredBuffer particleBuffer{};
	StructuredBuffer addParticleBuffer[2]{};
	StructuredBuffer deadParticleBuffer{};


	int addParticleCount;
	uint32_t instanceCount;

	RendererBuffer instanceCountBuffer{};
	RendererBuffer instanceCountStagingBuffer{};
	RendererBuffer deadParticleStagingBuffer{};

	ConstantBuffer optionBuffer;


	PixelShader pixelShader;

	std::vector<Binadble> PSshaderResources;
};

