#include "DrawCommand.h"
#include "../D3D11_Engine/Source/Math/Mathf.h"
#include <d3d11.h>

MeshDrawCommand::MeshDrawCommand() = default;
MeshDrawCommand::~MeshDrawCommand() = default;



Matrix UIMatrixHelper::MakeUIMatrix(size_t positionX,
						   size_t positionY,
						   size_t width,
						   size_t height,
						   size_t screenWidth,
						   size_t screenHeight,
						   float rotation_degree
						   )
{

	// 변환 행렬 초기화
	Matrix transform = Matrix::Identity;

	// 스케일과 변환 적용
	if (rotation_degree == 0.f)
	{
		transform = Matrix::CreateScale(width, height, 1.0f)
			* Matrix::CreateTranslation(positionX, positionY, 0.0f)
			* DirectX::XMMatrixOrthographicOffCenterLH(0.0f, screenWidth, screenHeight, 0.f, 0, 1);
	}
	else
	{
		transform =
			Matrix::CreateScale(width, height, 1.0f)
			* Matrix::CreateRotationZ(rotation_degree * Mathf::Deg2Rad)
			* Matrix::CreateTranslation(positionX, positionY, 0.0f)
			* DirectX::XMMatrixOrthographicOffCenterLH(0.0f, screenWidth, screenHeight, 0.f, 0, 1);
	}
	return transform;
}

Matrix UIMatrixHelper::MakeUIMatrixYFlip(size_t positionX, size_t positionY, long long width, long long height, size_t screenWidth, size_t screenHeight, float rotation_degree)
{
	// 변환 행렬 초기화
	Matrix transform = Matrix::Identity;

	// 스케일과 변환 적용
	if (rotation_degree == 0.f)
	{
		transform = Matrix::CreateScale(width, -height, 1.0f)
			* Matrix::CreateTranslation(positionX, positionY, 0.0f)
			* DirectX::XMMatrixOrthographicOffCenterLH(0.0f, screenWidth, screenHeight, 0.f, 0, 1);
	}
	else
	{
		transform =
			Matrix::CreateScale(width, -height, 1.0f)
			* Matrix::CreateRotationZ(rotation_degree * Mathf::Deg2Rad)
			* Matrix::CreateTranslation(positionX, positionY, 0.0f)
			* DirectX::XMMatrixOrthographicOffCenterLH(0.0f, screenWidth, screenHeight, 0.f, 0, 1);
	}
	return transform;
}

DebugMeshDrawCommand::DebugMeshDrawCommand()
{
}

DebugMeshDrawCommand::~DebugMeshDrawCommand()
{
}
