#include "Component/Collider/CapsuleCollider.h"
#include "Math/Mathf.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Physics/PhysicsManager.h"


CapsuleCollider::~CapsuleCollider()
{
	DetachShapeToActor();
}

void CapsuleCollider::SetSize(float body_height, float radius)
{
	this->body_height = body_height;
	this->radius = radius;
	UpdateShape();
	AttachShapeToActor();
}

void CapsuleCollider::SetSizeFollowingMesh()
{
	BoundingBox bb = GetGameObject().GetBBToWorld();

	radius = std::max<float>(bb.Extents.x, bb.Extents.z);
	if (radius <= 0.0f) radius = 0.1f;
	body_height = (bb.Extents.y - radius) * 2;
	if (body_height <= 0.0f) body_height = 0.1f; // 이게 맞나?

	UpdateShape();
}

void CapsuleCollider::AttachShapeToActor()
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

void CapsuleCollider::UpdateShape()
{
	if (shape != nullptr)
	{
		DetachShapeToActor();
		shape->release();
	}

	px::PxPhysics* physics = PhysicsManager::GetInstance().GetPhysics();

	Vector3 scale = GetGameObject().transform.GetScale();
	px::PxCapsuleGeometry geometry{ radius * std::max<float>(scale.x, scale.z), body_height * 0.5f * scale.y };

	PhysicsMaterial* pm = GetPhysicsMaterial();
	if (pm == nullptr)
	{
		pm = PhysicsManager::GetInstance().GetDefaultPhysicsMaterial();
	}
	shape = physics->createShape(geometry, *pm->GetPxMaterial(), true);

	// 기본 선 채로
	px::PxTransform relative{ px::PxQuat{px::PxHalfPi, px::PxVec3(0, 0, 1) } };
	shape->setLocalPose(relative);
	shape->userData = static_cast<void*>(this);
}

void CapsuleCollider::DetachShapeToActor()
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

void CapsuleCollider::SetIsTrigger(bool is_trigger)
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

void CapsuleCollider::Awake()
{
	PhysicsManager::OnAddCollider(&GetGameObject(), this);
}
