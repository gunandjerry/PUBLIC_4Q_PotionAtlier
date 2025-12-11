#include "CharacterController.h"
#include "Core/TimeSystem.h"
#include "Physics/PhysicsActor/PhysicsActor.h"
#include "Utility/SerializedUtility.h"
#include  <framework.h>

using namespace TimeSystem;

float CharacterController::GRAVITY_MULTIPLIER{ 75000.0f };

void CharacterController::EnableCollider()
{
	controller->getActor()->attachShape(*capsule);
}

void CharacterController::DisableCollider()
{
	controller->getActor()->detachShape(*capsule);
}

void CharacterController::Awake()
{
	UpdateShape();
}

void CharacterController::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, height);
	Binary::Write::data(ofs, radius);
}

void CharacterController::Deserialized(std::ifstream& ifs)
{
	height = Binary::Read::data<float>(ifs);
	radius = Binary::Read::data<float>(ifs);

	UpdateShape();
}

CharacterController::~CharacterController()
{
	if (PhysicsActor* actor = GetGameObject().GetPhysicsActor(); actor != nullptr)
	{
		// actor 직접 구해서 release하면 터지는데 controller 지우면 지가 알아서 죽이나???
		//auto* a = actor->GetActor();
		//actor->GetActor()->release();
		(*GetGameObject().GetPhysicsActorAddress()) = nullptr;
	}

	if (controller != nullptr) controller->release();
}

void CharacterController::SetSizeAutomatically()
{
	BoundingBox bb{};
	const auto& map = Utility::CollectMeshComponents(&gameObject);
	for (auto& [key, vec] : map)
	{
		bb.CreateMerged(bb, bb, vec[0]->gameObject.GetBBToWorld());
	}

	radius = std::max<float>(bb.Extents.x, bb.Extents.z);
	if (radius <= 0.0f) radius = 0.1f;
	height = (bb.Extents.y - radius) * 2;
	if (height <= 0.0f) height = 0.1f; // 이게 맞나?

	UpdateShape();
}

void CharacterController::SetSize(float height, float radius)
{
	this->height = height;
	this->radius = radius;
	UpdateShape();
}

void CharacterController::UpdateShape()
{
	if (controller != nullptr)
	{
		//controller->release();
		(*GetGameObject().GetPhysicsActorAddress()) = nullptr;
	}
	if (GetGameObject().GetPhysicsActor() != nullptr)
	{
		delete GetGameObject().GetPhysicsActor();
	}

	if (material == nullptr)
	{
		material = PhysicsManager::GetInstance().GetDefaultPhysicsMaterial();
	}

	const Vector3& scale = GetGameObject().transform.GetScale();
	px::PxCapsuleControllerDesc desc{};
	desc.height = height * scale.y;
	desc.radius = radius * std::max<float>(scale.x, scale.z);
	desc.position = px::PxExtendedVec3{ 0, 0, 0 };
	desc.material = material->GetPxMaterial();
	desc.density = density;
	desc.contactOffset = contact_offset;
	desc.slopeLimit = slope_limit;
	desc.stepOffset = step_offset;

	px::PxControllerManager* controller_manager = PhysicsManager::GetInstance().GetPhysicsScene()->GetControllerManager();
	if (controller == nullptr)
	{
		controller = static_cast<px::PxCapsuleController*>(controller_manager->createController(desc));
		controller->getActor()->userData = static_cast<void*>(&GetGameObject());
	}
	else
	{
		controller->setRadius(desc.radius);
		controller->setHeight(desc.height);
	}
	SetPosition(GetGameObject().transform.GetPosition());

	controller->getActor()->getShapes(&capsule, 1, 0);

	(*GetGameObject().GetPhysicsActorAddress()) = new PhysicsActor(this);
}

void CharacterController::SetPosition(Vector3 position)
{
	controller->setPosition(px::PxExtendedVec3{ position.x, position.y, position.z });
}

Vector3 CharacterController::GetPosition()
{
	px::PxExtendedVec3 pos = controller->getPosition();
	return Vector3{ (float)pos.x, (float)pos.y, (float)pos.z };
}

void CharacterController::Move(Vector3 movement)
{
	//float dt = Time::GetDeltaTime();
	//px::PxControllerCollisionFlags collisionFlags = controller->move(movement * dt, 0.0001f, dt, px::PxControllerFilters());

	AddVelocity(movement);
}

void CharacterController::Jump(float power)
{
	AddVelocity({ 0, power, 0 });
}

void CharacterController::AddVelocity(Vector3 velocity)
{
	this->velocity += velocity;
}

void CharacterController::SetVelocity(Vector3 velocity)
{
	this->velocity = velocity;
}

void CharacterController::CalculateVelocity()
{
	float dt = Time.DeltaTime;
	velocity += gravity * GRAVITY_MULTIPLIER * dt;
	Vector3 movement = velocity * dt;

	px::PxControllerCollisionFlags collisionFlags = controller->move(PhysicsManager::ConvertToPxVec(movement), 0.001f, dt, px::PxControllerFilters());

	if (collisionFlags & px::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		velocity.y = 0.0f;
		is_ground = true;
	}
	else
	{
		is_ground = false;
	}
}

void CharacterController::ResetMovementVelocity()
{
	velocity.x = 0.0f;
	velocity.z = 0.0f;
}