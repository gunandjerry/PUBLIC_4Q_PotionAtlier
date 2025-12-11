#include "SimpleBoneMeshRender.h"
#include <Light\SimpleDirectionalLight.h>
#include <Manager/ResourceManager.h>
#include <Math/Mathf.h>
#include <Utility/SerializedUtility.h>
#include <GameObject/Bone/BoneObject.h>
#include <Utility/ExceptionUtility.h>


SimpleBoneMeshRender::SimpleBoneMeshRender()
{

}

SimpleBoneMeshRender::~SimpleBoneMeshRender()
{
	
}

#define SERIALIZED_VERSION 1
#define DESERIALIZED_VERSION 1
void SimpleBoneMeshRender::Serialized(std::ofstream& ofs)
{
	if (typeid(gameObject) == typeid(GameObject))
		return;

	using namespace Binary;
	MeshRender::Serialized(ofs);

	//offsetMatrix
	std::wstring offsetMatrixKey = gameObject.Name + utfConvert::utf8_to_wstring(GetMeshID());
	Write::wstring(ofs, offsetMatrixKey);
	size_t offsetMatrixCount = offsetMatrices->data.size();
	Write::data(ofs, offsetMatrixCount);
	Write::data(ofs, offsetMatrices->data.data(), sizeof(Matrix) * offsetMatrixCount);
}

void SimpleBoneMeshRender::Deserialized(std::ifstream& ifs)
{
	if (typeid(gameObject) == typeid(GameObject))
		return;

	using namespace Binary;
	MeshRender::Deserialized(ifs);

	//offsetMatrix
	std::wstring offsetMatrixKey = Read::wstring(ifs);
	offsetMatrices = GetResourceManager<OffsetMatrices>().GetResource(offsetMatrixKey.c_str());
	size_t offsetMatrixCount = Read::data<size_t>(ifs);
	if (offsetMatrices.use_count() == 1)
	{
		offsetMatrices->data.resize(offsetMatrixCount);
		Read::data(ifs, offsetMatrices->data.data(), sizeof(Matrix) * offsetMatrixCount);
	}
	else
	{
		ifs.seekg(sizeof(Matrix) * offsetMatrixCount, std::ios::cur);
	}
		
	DeserializedListVec.push_back(this);
}

void SimpleBoneMeshRender::EndDeserialized()
{
	for (auto& item : SimpleBoneMeshRender::DeserializedListVec)
	{
		item->AddBonesFromRoot();
	}
	SimpleBoneMeshRender::DeserializedListVec.clear();
	SimpleBoneMeshRender::DeserializedListVec.shrink_to_fit();
}

void SimpleBoneMeshRender::Awake()
{

}

void SimpleBoneMeshRender::UpdateMeshDrawCommand()
{
	Matrix temp{};

	//카메라 컬링 확인 필요할듯..?
	//본 행렬 업데이트
	for (int i = 0; i < boneList.size(); i++)
	{
		temp = offsetMatrices->data[i] * boneList[i]->GetBoneMatrix();
		matrixPallete.MatrixPalleteArray[i] = XMMatrixTranspose(temp);

		temp = XMMatrixInverse(nullptr, temp);
		temp = Utility::XMMatrixIsNaN(temp) ? Matrix() : temp;
		matrixPallete.BoneWIT[i] = temp;
	}
	matrixPalleteConstBuffer.Set(matrixPallete);
	meshDrawCommand.meshData.shaderResources.clear();
	meshDrawCommand.meshData.shaderResources.push_back(
		Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 3,
			.bind = (ID3D11Buffer*)matrixPalleteConstBuffer
		}
	);
}

void SimpleBoneMeshRender::LoadMeshResource(std::filesystem::path& meshResourcePath)
{
	using namespace Binary;
	meshBufferResource = GetResourceManager<MeshBufferResource>().GetResource(meshResourcePath.c_str());
	bool isResource = meshBufferResource.use_count() > 1;
	if (isResource)
	{
		meshDrawCommand.meshData.indexCounts = meshBufferResource->indexCounts;
		meshDrawCommand.meshData.indexBuffer.Load(meshBufferResource->indexBufferResource);
		meshDrawCommand.meshData.vertexStride = meshBufferResource->vertexStride;
		meshDrawCommand.meshData.vertexBuffer.Load(meshBufferResource->vertexBufferResource);
	}
	else
	{
		std::ifstream ifs(meshResourcePath, std::ios::binary);
		if (ifs.is_open())
		{
			size_t indicesSize = Read::data<size_t>(ifs);
			indices.resize(indicesSize);
			ifs.read(reinterpret_cast<char*>(indices.data()), sizeof(decltype(indices[0])) * indicesSize);

			size_t verticesSize = Read::data<size_t>(ifs);
			vertices.resize(verticesSize);
			ifs.read(reinterpret_cast<char*>(vertices.data()), sizeof(Vertex) * verticesSize);

			if (!indices.empty() && !vertices.empty())
				CreateMesh();
		}
		ifs.close();
		meshBufferResource->indexCounts = meshDrawCommand.meshData.indexCounts;
		meshBufferResource->indexBufferResource = (ID3D11Buffer*)meshDrawCommand.meshData.indexBuffer;
		meshBufferResource->vertexStride = meshDrawCommand.meshData.vertexStride;
		meshBufferResource->vertexBufferResource = (ID3D11Buffer*)meshDrawCommand.meshData.vertexBuffer;
	}
}

void SimpleBoneMeshRender::SaveMeshResource(std::filesystem::path& meshResourcePath)
{
	using namespace Binary;

	std::filesystem::path& savePath = meshResourcePath;
	if (!savePath.empty())
	{
		if (!std::filesystem::exists(savePath))
		{
			std::filesystem::create_directories(savePath.parent_path());
		}
		ID3D11Device* pDevice = RendererUtility::GetDevice();
		ComPtr<ID3D11DeviceContext> pDeviceContext;
		pDevice->GetImmediateContext(&pDeviceContext);

		std::ofstream ofs(savePath, std::ios::binary | std::ios::trunc);

		D3D11_BUFFER_DESC bd{};
		if (ID3D11Buffer* indexBuffer = (ID3D11Buffer*)meshDrawCommand.meshData.indexBuffer)
		{
			indexBuffer->GetDesc(&bd);
			indices.resize(bd.ByteWidth / sizeof(UINT));
			Utility::RetrieveIndexBufferData(pDeviceContext.Get(), pDevice, indexBuffer, indices.data(), bd.ByteWidth);
			Write::data(ofs, indices.size());
			ofs.write(reinterpret_cast<const char*>(indices.data()), bd.ByteWidth);
			indices.clear();
		}
		else
		{
			Write::data(ofs, indices.size());
		}

		if (ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)meshDrawCommand.meshData.vertexBuffer)
		{
			vertexBuffer->GetDesc(&bd);
			vertices.resize(bd.ByteWidth / sizeof(Vertex));
			Utility::RetrieveVertexBufferData(pDeviceContext.Get(), pDevice, vertexBuffer, vertices.data(), bd.ByteWidth);
			Write::data(ofs, vertices.size());
			ofs.write(reinterpret_cast<const char*>(vertices.data()), bd.ByteWidth);
			vertices.clear();
		}
		else
		{
			Write::data(ofs, vertices.size());
		}

		indices.shrink_to_fit();
		vertices.shrink_to_fit();

		ofs.close();
	}
}

void SimpleBoneMeshRender::CreateMesh()
{
	using namespace Utility;
	if (vertices.empty() || indices.empty())
		return;

	ID3D11Device* pDevice = RendererUtility::GetDevice();

	//버텍스 버퍼 생성
	meshDrawCommand.meshData.vertexStride = sizeof(Vertex);

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(Vertex) * vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices.data();

	ComPtr<ID3D11Buffer> vertexBuffer;
	CheckHRESULT(pDevice->CreateBuffer(&bd, &vbData, &vertexBuffer));
	meshDrawCommand.meshData.vertexBuffer.Load(vertexBuffer);

	//인덱스 버퍼 생성
	meshDrawCommand.meshData.indexCounts = indices.size();
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(UINT) * indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices.data();

	ComPtr<ID3D11Buffer> indexBuffer;
	CheckHRESULT(pDevice->CreateBuffer(&bd, &ibData, &indexBuffer));
	meshDrawCommand.meshData.indexBuffer.Load(indexBuffer);

	//Create bounding box
	BoundingBox box;
	box.CreateFromPoints(box, vertices.size(), reinterpret_cast<XMFLOAT3*>(vertices.data()), sizeof(Vertex));
	if (
		gameObject.Bounds.Extents.x > Mathf::Epsilon && 
		gameObject.Bounds.Extents.y > Mathf::Epsilon &&
		gameObject.Bounds.Extents.z > Mathf::Epsilon	
		)
	{
		BoundingBox::CreateMerged(gameObject.Bounds, gameObject.Bounds, box);
	}
	else
		gameObject.Bounds = box;

	if (Transform* root = gameObject.transform.RootParent)
	{
		box.Transform(box, transform.scale.x, transform.rotation, transform.position);
		if (
			root->gameObject.Bounds.Extents.x > Mathf::Epsilon && 
			root->gameObject.Bounds.Extents.y > Mathf::Epsilon &&
			root->gameObject.Bounds.Extents.z > Mathf::Epsilon
			)
		{
			BoundingBox::CreateMerged(root->gameObject.Bounds, root->gameObject.Bounds, box);
		}
		else
			root->gameObject.Bounds = box;
	}

	vertices.clear();
	indices.clear();

	vertices.shrink_to_fit();
	indices.shrink_to_fit();
}

void SimpleBoneMeshRender::AddBonesFromRoot()
{
	if (gameObject.transform.RootParent)
	{
		Transform* root = gameObject.transform.RootParent;
		boneList.clear();
		std::stack<Transform*> transformStack;
		for (unsigned int i = 0; i < root->GetChildCount(); i++)
		{
			transformStack.push(root->GetChild(i));
		}
		while (!transformStack.empty())
		{
			Transform* cur = transformStack.top();
			transformStack.pop();
			if(typeid(BoneObject) == typeid(cur->gameObject))
			{
				BoneObject& curBone = static_cast<BoneObject&>(cur->gameObject);
				if (curBone.myIndex < 0)
				{
					__debugbreak(); //인덱스가 잘못됨.
					throw_GameObject("Invalid bone index", &this->gameObject);
				}
				if (boneList.size() <= curBone.myIndex)
				{
					boneList.resize((size_t)curBone.myIndex + 1);			
				}
				boneList[curBone.myIndex] = &curBone;
			}			 

			for (unsigned int i = 0; i < cur->GetChildCount(); i++)
			{
				transformStack.push(cur->GetChild(i));
			}
		}
	}
}
