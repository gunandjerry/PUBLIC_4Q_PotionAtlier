#include "Component/Collider/MeshCollider.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsManager.h"
#include <Utility/ImguiHelper.h>
#include "Component/Render/PBRMeshRender.h"
#include "RendererBuffer.h"
#include "Utility/SerializedUtility.h"
#include "GameObject/Base/GameObject.h"


MeshCollider::~MeshCollider()
{
	PhysicsActor** _actor = GetGameObject().GetPhysicsActorAddress();
	//(*_actor)->GetActor()->release();
	*_actor = nullptr;
}

void MeshCollider::CreateShapeFromChildMesh()
{
	unsigned int child_num = transform.GetChildCount();
	PBRMeshRender* renderer{ nullptr };
	Vector3 scale{ 1,1,1 };

	for (int i = 0; i < child_num; ++i)
	{
		auto* child = transform.GetChild(i);
		renderer = child->gameObject.IsComponent<PBRMeshRender>();
		if (renderer != nullptr)
		{
			scale = child->GetLocalScale();
			break;
		}
	}

	if (renderer == nullptr)
	{
		printf("메쉬 없는데?\n");
		return;
	}

	std::vector<SimpleMeshRender::Vertex> vertices;

	MeshData* meshData = &renderer->GetMeshDrawCommand()->meshData;
	ID3D11Buffer* vb = meshData->vertexBuffer;
	ID3D11Buffer* ib = meshData->indexBuffer;

	ID3D11Device* device = RendererUtility::GetDevice();
	ID3D11DeviceContext* context;
	device->GetImmediateContext(&context);


	D3D11_BUFFER_DESC vertexBufferDesc;
	vb->GetDesc(&vertexBufferDesc);

	ID3D11Buffer* copyVertexBuffer;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	vertexBufferDesc.Usage = D3D11_USAGE_STAGING;
	vertexBufferDesc.BindFlags = 0;
	HRESULT hr = device->CreateBuffer(&vertexBufferDesc, nullptr, &copyVertexBuffer);
	context->CopyResource(copyVertexBuffer, vb);

	D3D11_MAPPED_SUBRESOURCE mappedResourceVertex;
	hr = context->Map(copyVertexBuffer, 0, D3D11_MAP_READ, 0, &mappedResourceVertex);

	size_t vertexCount = vertexBufferDesc.ByteWidth / sizeof(SimpleMeshRender::Vertex);
	vertices.resize(vertexCount);
	memcpy(vertices.data(), mappedResourceVertex.pData, vertexBufferDesc.ByteWidth);

	context->Unmap(copyVertexBuffer, 0);






	D3D11_BUFFER_DESC indexBufferDesc;
	ib->GetDesc(&indexBufferDesc);

	ID3D11Buffer* copyIndexBuffer;
	indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	indexBufferDesc.Usage = D3D11_USAGE_STAGING;
	indexBufferDesc.BindFlags = 0;
	hr = device->CreateBuffer(&indexBufferDesc, nullptr, &copyIndexBuffer);
	context->CopyResource(copyIndexBuffer, ib);

	D3D11_MAPPED_SUBRESOURCE mappedResourceIndex;
	hr = context->Map(copyIndexBuffer, 0, D3D11_MAP_READ, 0, &mappedResourceIndex);

	size_t indexCount = indexBufferDesc.ByteWidth / sizeof(UINT);
	indices.resize(indexCount);
	memcpy(indices.data(), mappedResourceIndex.pData, indexBufferDesc.ByteWidth);

	context->Unmap(copyIndexBuffer, 0);



	
	pxVertices.resize(vertices.size());
	for (int i = 0; i < vertices.size(); ++i)
	{
		px::PxVec3 _pos = { vertices[i].position.x, vertices[i].position.y, vertices[i].position.z };
		_pos.x *= scale.x;
		_pos.y *= scale.y;
		_pos.z *= scale.z;

		pxVertices[i] = _pos;
	}
}

void MeshCollider::BakeOnStart()
{
	if (!pxVertices.empty() && !indices.empty())
	{
		Bake();
	}
}

void MeshCollider::Bake()
{
	// test
	//std::reverse(indices.begin(), indices.end());

	/*pxVertices.clear();
	pxVertices.push_back({ 100.0f, 0, 100.0f });
	pxVertices.push_back({ -100.0f, 0, 100.0f });
	pxVertices.push_back({ -100.0f, 0, -100.0f });
	pxVertices.push_back({ 100.0f, 0, -100.0f });
	indices.clear();
	indices = std::vector<px::PxU32>{
		0, 2, 1,
		0, 3, 2
	};*/



	px::PxTriangleMeshDesc meshDesc{};
	meshDesc.points.count = pxVertices.size();
	meshDesc.points.stride = sizeof(px::PxVec3);
	meshDesc.points.data = pxVertices.data();
	meshDesc.triangles.count = indices.size() / 3;
	meshDesc.triangles.stride = 3 * sizeof(px::PxU32);
	meshDesc.triangles.data = indices.data();

	px::PxTolerancesScale toleranceScale{};
	px::PxCookingParams params(toleranceScale);

	// 이건 외않됞대?
	//params.meshPreprocessParams |= px::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	//params.meshPreprocessParams |= px::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	//params.midphaseDesc.mBVH33Desc.meshCookingHint = px::PxMeshCookingHint::eCOOKING_PERFORMANCE;

	px::PxPhysics* physics = PhysicsManager::GetInstance().GetPhysics();
	//px::PxTriangleMesh* triangleMesh = PxCreateTriangleMesh(params, meshDesc, physics->getPhysicsInsertionCallback());
	px::PxDefaultMemoryOutputStream writeBuffer{};
	px::PxTriangleMeshCookingResult::Enum result{};
	bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
	if (!status)
	{
		printf("만들기 싫은데?\n");
		return;
	}

	px::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	px::PxTriangleMesh* triangleMesh = physics->createTriangleMesh(readBuffer);

	PhysicsMaterial* pm = GetPhysicsMaterial();
	if (pm == nullptr)
	{
		pm = PhysicsManager::GetInstance().GetDefaultPhysicsMaterial();
	}
	shape = physics->createShape(px::PxTriangleMeshGeometry(triangleMesh), *pm->GetPxMaterial(), true);
	
	AttachShapeToActor();
}
void MeshCollider::AttachShapeToActor()
{
	if (shape == nullptr)
	{
		//Debug::LogError("PxShape not found.");
		return;
	}

	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("PxActor not found.");
		return;
	}

	actor->GetActor()->attachShape(*shape);
}

void MeshCollider::DetachShapeToActor()
{
	if (shape == nullptr)
	{
		//Debug::LogError("PxShape not found.");
		return;
	}

	PhysicsActor* actor = GetGameObject().GetPhysicsActor();
	if (actor == nullptr)
	{
		//Debug::LogError("PxActor not found.");
		return;
	}

	auto* aa = actor->GetActor();

	actor->GetActor()->detachShape(*shape);
}

void MeshCollider::SetIsTrigger(bool is_trigger)
{
	this->is_trigger = is_trigger;
	if (shape == nullptr)
	{
		//Debug::LogError("PxShape not found.");
		return;
	}

	shape->setFlag(px::PxShapeFlag::eSIMULATION_SHAPE, !is_trigger);
	shape->setFlag(px::PxShapeFlag::eTRIGGER_SHAPE, is_trigger);
}

void MeshCollider::Awake()
{
	PhysicsManager::OnAddCollider(&GetGameObject(), this);
}

void MeshCollider::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, pxVertices.size());
	Binary::Write::std_vector<px::PxVec3>(ofs, pxVertices);
	Binary::Write::data(ofs, indices.size());
	Binary::Write::std_vector(ofs, indices);
}

void MeshCollider::Deserialized(std::ifstream& ifs)
{
	pxVertices.resize(Binary::Read::data<size_t>(ifs));
	Binary::Read::std_vector(ifs, pxVertices);
	indices.resize(Binary::Read::data<size_t>(ifs));
	Binary::Read::std_vector(ifs, indices);

	BakeOnStart();
}

void MeshCollider::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("MeshCollider"))
	{
		if (ImGui::Button("Bake Mesh"))
		{
			CreateShapeFromChildMesh();
			Bake();
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}