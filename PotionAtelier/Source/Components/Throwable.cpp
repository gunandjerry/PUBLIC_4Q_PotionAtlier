#include "Throwable.h"
#include "PlayerController.h"
#include "Core/TimeSystem.h"

using namespace TimeSystem;

void Throwable::Awake()
{
}

void Throwable::Update()
{
	if (!on_move) return;

	t += Time.DeltaTime / hold_time;

	if (t >= 1.0f)
	{
		t = 1.0f;
		on_move = false;
	}

	transform.localPosition = Vector3::Lerp(current_position, goal_position, t);
}

void Throwable::OnFocusIn(PlayerController* controller)
{
	transform.SetScale({ 1.1f, 1.1f, 1.1f });
}

void Throwable::OnFocusOut(PlayerController* controller)
{
	transform.SetScale({ 1.0f, 1.0f, 1.0f });
}

bool Throwable::OnInteract(PlayerController* controller)
{
	auto&& vec = gameObject.GetEveryCollider();
	for (auto* c : vec)
	{
		c->DetachShapeToActor();
		c->Enable = false;
	}

	XMVECTOR parent_rotation_inverse = XMQuaternionInverse(controller->transform.rotation);
	current_position = XMVector3Rotate(transform.position - controller->transform.position, parent_rotation_inverse);
	
	distance_to_goal = Vector3::Distance(goal_position, current_position);
	
	transform.SetParent(controller->transform);
	on_move = true;
	transform.SetLocalPosition(current_position);


	// 
	return true;
}

void Throwable::Throw(Vector3 direction)
{
	Vector3 worldPos = transform.GetPosition();
	Quaternion worldRot = transform.GetRotation();
	
	transform.SetParent(nullptr);
	transform.SetPosition(worldPos);
	transform.SetRotation(worldRot);

	auto&& vec = gameObject.GetEveryCollider();
	for (auto* c : vec)
	{
		c->AttachShapeToActor();
		c->Enable = true;
	}

	Rigidbody* rb = gameObject.IsComponent<Rigidbody>();
	if (rb == nullptr)
	{
		rb = &gameObject.AddComponent<Rigidbody>();
	}
	direction.y += 0.5f;
	rb->AddForce(direction * 20.0f, ForceMode::Impulse);
}
