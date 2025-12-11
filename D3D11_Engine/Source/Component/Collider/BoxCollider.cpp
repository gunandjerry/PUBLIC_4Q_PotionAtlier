#include "Component/Collider/BoxCollider.h"
#include "Math/Mathf.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsManager.h"
#include <Utility/ImguiHelper.h>
#include "Utility/SerializedUtility.h"
#include "Component/Render/MeshRender.h"


void BoxCollider::Awake()
{
	PhysicsManager::OnAddCollider(&GetGameObject(), this);
	prev_anchor = anchor;
	prev_size = size;
}

void BoxCollider::Update()
{
	if (prev_anchor != anchor)
	{
		prev_anchor = anchor;
		UpdateAnchor();
	}
	if (prev_size != size)
	{
		prev_size = size;
		SetSize(size.x, size.y, size.z);
	}
	if (debug_draw)
	{
		DrawBounds();
	}
}

BoxCollider::~BoxCollider()
{
	DetachShapeToActor();
}

void BoxCollider::CreateShapeFromChildMesh()
{
	BoundingBox bb{};

	unsigned int child_num = transform.GetChildCount();
	for (int i = 0; i < child_num; ++i)
	{
		auto* child = transform.GetChild(i);
		BoundingBox _bb = child->gameObject.GetBBToWorld();  // BB 구할 때 이미 스케일 곱함
		bb.CreateMerged(bb, bb, _bb);
	}

	size.x = bb.Extents.x * 2.0f;
	size.y = bb.Extents.y * 2.0f;
	size.z = bb.Extents.z * 2.0f;
	anchor.x = bb.Center.x;
	anchor.y = bb.Center.y;
	anchor.z = bb.Center.z;

	size.x = size.x <= 0.0f ? 0.1f : size.x;
	size.y = size.y <= 0.0f ? 0.1f : size.y;
	size.z = size.z <= 0.0f ? 0.1f : size.z;

	UpdateShape();
	AttachShapeToActor();
}

void BoxCollider::SetSize(float width, float height, float depth)
{
	size = { width, height, depth };
	UpdateShape();
	AttachShapeToActor();
}

void BoxCollider::SetSizeFollowingMesh()
{
	BoundingBox bb = GetGameObject().GetBBToWorld(); // BB 구할 때 이미 스케일 곱함

	size.x = bb.Extents.x * 2.0f;
	size.y = bb.Extents.y * 2.0f;
	size.z = bb.Extents.z * 2.0f;

	if (size.x == 0.0f || size.y == 0.0f || size.z == 0.0f)
	{
		size = { 1, 1, 1 };
	}

	UpdateShape();
	AttachShapeToActor();
}

void BoxCollider::DrawBounds(Color color)
{
	DebugMeshDrawCommand command;
	BoundingOrientedBox bob;
	bob.Extents.x = size.x * 0.5f;
	bob.Extents.y = size.y * 0.5f;
	bob.Extents.z = size.z * 0.5f;
	bob.Transform(bob, transform.GetWM());
	bob.Center = bob.Center + anchor;
	command.boundingBox = bob;
	command.color = Color{ 0, 1, 0, 1 };
	command.type = EDebugMeshDraw::Type::Box;

	D3D11_GameApp::GetRenderer().AddDrawCommand(command);

	/*BoundingOrientedBox bob;
	bob.Center = { 0, 0, 0 };
	bob.Extents.x = width * 0.5f;
	bob.Extents.y = height * 0.5f;
	bob.Extents.z = depth * 0.5f;
	bob.Transform(bob, GetOwner()->transform->GetWorldTransformMatrix());

	Gizmo::DrawBounds(bob, color);*/
}

void BoxCollider::AttachShapeToActor()
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

void BoxCollider::UpdateShape()
{
	if (shape != nullptr)
	{
		DetachShapeToActor();
		shape->release();
		shape = nullptr;
	}

	px::PxPhysics* physics = PhysicsManager::GetInstance().GetPhysics();

	Vector3 real_half_size{ size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
	Vector3 scale = GetGameObject().transform.GetScale();
	real_half_size *= GetGameObject().transform.GetScale();

	real_half_size.x = real_half_size.x <= 0.0f ? 0.1f : real_half_size.x;
	real_half_size.y = real_half_size.y <= 0.0f ? 0.1f : real_half_size.y;
	real_half_size.z = real_half_size.z <= 0.0f ? 0.1f : real_half_size.z;



	px::PxBoxGeometry geometry{ PhysicsManager::ConvertToPxVec(real_half_size) };
	PhysicsMaterial* pm = GetPhysicsMaterial();
	if (pm == nullptr)
	{
		pm = PhysicsManager::GetInstance().GetDefaultPhysicsMaterial();
	}
	shape = physics->createShape(geometry, *pm->GetPxMaterial(), true);

	px::PxTransform localTransform{ PhysicsManager::ConvertToPxVec(anchor) };
	shape->setLocalPose(localTransform);

	// geometry에 0 담기면 shape 생성이 아예 무시되던데?
	if (shape == nullptr)
	{
		//Debug::LogError("Shape not found.");
		return;
	}
	shape->userData = static_cast<void*>(this);
}

void BoxCollider::DetachShapeToActor()
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
	UINT num = aa->getNbShapes();

	actor->GetActor()->detachShape(*shape);
}

void BoxCollider::SetIsTrigger(bool is_trigger)
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

void BoxCollider::UpdateAnchor()
{
	px::PxTransform localTransform{ PhysicsManager::ConvertToPxVec(anchor) };
	shape->setLocalPose(localTransform);
}

void BoxCollider::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("BoxCollider"))
	{
		ImGui::Checkbox("Draw Bounds", &debug_draw);
		ImGui::DragFloat3("Anchor", &anchor.x, 0.05f);
		ImGui::DragFloat3("Size", &size.x, 0.05f, 0.001f);

		if (ImGui::Button("Adjust size automatically"))
		{
			CreateShapeFromChildMesh();
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void BoxCollider::Serialized(std::ofstream& ofs)
{
	Binary::Write::Vector3(ofs, size);
	Binary::Write::Vector3(ofs, prev_anchor);
	Binary::Write::Vector3(ofs, prev_size);
	Binary::Write::Vector3(ofs, anchor);
}

void BoxCollider::Deserialized(std::ifstream& ifs)
{
	size = Binary::Read::Vector3(ifs);
	prev_anchor = Binary::Read::Vector3(ifs);
	prev_size = Binary::Read::Vector3(ifs);

	anchor = Binary::Read::Vector3(ifs);
	UpdateShape();
	AttachShapeToActor();
}

