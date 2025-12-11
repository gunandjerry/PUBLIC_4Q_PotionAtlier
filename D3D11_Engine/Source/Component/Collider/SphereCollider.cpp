#include "Component/Collider/SphereCollider.h"
#include "Math/Mathf.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsManager.h"
#include <Utility/ImguiHelper.h>
#include "Utility/SerializedUtility.h"


SphereCollider::~SphereCollider()
{
	DetachShapeToActor();
}

void SphereCollider::SetSize(float radius)
{
	this->radius = radius;
	UpdateShape();
	AttachShapeToActor();
}

void SphereCollider::SetSizeFollowingMesh()
{
	BoundingBox bb = GetGameObject().GetBBToWorld();

	radius = std::max<float>({ bb.Extents.x, bb.Extents.y, bb.Extents.z });
	UpdateShape();
}

void SphereCollider::DrawBounds(Color color)
{
	DebugMeshDrawCommand command;
	BoundingSphere bs;
	bs.Radius = radius;
	bs.Transform(bs, transform.GetWM());
	bs.Center = bs.Center + anchor;
	command.boundingSphere = bs;
	command.color = Color{ 0, 1, 0, 1 };
	command.type = EDebugMeshDraw::Type::Sphere;

	D3D11_GameApp::GetRenderer().AddDrawCommand(command);
	/*BoundingSphere bs;
	bs.Center = { 0, 0, 0 };
	bs.Radius = radius;
	bs.Transform(bs, GetOwner()->transform->GetWorldTransformMatrix());

	Gizmo::DrawBounds(bs, color);*/
}

void SphereCollider::AttachShapeToActor()
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

void SphereCollider::UpdateShape()
{
	if (shape != nullptr)
	{
		DetachShapeToActor();
		shape->release();
	}

	px::PxPhysics* physics = PhysicsManager::GetInstance().GetPhysics();

	Vector3 scale = GetGameObject().transform.GetScale();
	px::PxSphereGeometry geometry{ radius * std::max<float>({scale.x, scale.y, scale.z}) };

	PhysicsMaterial* pm = GetPhysicsMaterial();
	if (pm == nullptr)
	{
		pm = PhysicsManager::GetInstance().GetDefaultPhysicsMaterial();
	}
	shape = physics->createShape(geometry, *pm->GetPxMaterial(), true);
	shape->userData = static_cast<void*>(this);
}

void SphereCollider::DetachShapeToActor()
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

	actor->GetActor()->detachShape(*shape);
}

void SphereCollider::SetIsTrigger(bool is_trigger)
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

void SphereCollider::UpdateAnchor()
{
	px::PxTransform localTransform{ PhysicsManager::ConvertToPxVec(anchor) };
	shape->setLocalPose(localTransform);
}

void SphereCollider::Awake()
{
	PhysicsManager::OnAddCollider(&GetGameObject(), this);
	prev_anchor = anchor;
	prev_radius = radius;
}

void SphereCollider::Update()
{
	if (prev_anchor != anchor)
	{
		prev_anchor = anchor;
		UpdateAnchor();
	}
	if (prev_radius != radius)
	{
		prev_radius = radius;
		SetSize(radius);
	}
	if (debug_draw)
	{
		DrawBounds();
	}
}

void SphereCollider::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("BoxCollider"))
	{
		ImGui::Checkbox("Draw Bounds", &debug_draw);
		ImGui::DragFloat3("Anchor", &anchor.x, 0.05f);
		ImGui::DragFloat("Radius", &radius);

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void SphereCollider::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, radius);
	Binary::Write::data(ofs, prev_radius);
	Binary::Write::Vector3(ofs, anchor);
	Binary::Write::Vector3(ofs, prev_anchor);
}

void SphereCollider::Deserialized(std::ifstream& ifs)
{
	radius = Binary::Read::data<float>(ifs);
	prev_radius = Binary::Read::data<float>(ifs);

	anchor = Binary::Read::Vector3(ifs);
	prev_anchor = Binary::Read::Vector3(ifs);

	UpdateShape();
	AttachShapeToActor();
}
