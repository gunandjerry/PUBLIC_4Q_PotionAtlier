#pragma once
#include "Physics/PhysXFramework.h"
#include "Component/Base/Component.h"
#include "Physics/PhysicsMaterial/PhysicsMaterial.h"


// Memo
// CharacterController 추가하면 PhysicsActor, CapsuleCollider를 자체적으로 생성하므로 Rigidbody나 Collider와 함께 추가하지 않도록 함.

class CharacterController : public Component
{
	PhysicsMaterial* material{ nullptr };
	px::PxCapsuleController* controller{ nullptr };
	px::PxShape* capsule{ nullptr };
	px::PxControllerFilters controller_filter{};

	float height{ 1.0f };
	float radius{ 1.0f };
	float density{ 100.0f };
	float contact_offset{ 0.05f };
	float slope_limit{ 0.2f };
	float step_offset{ 0.75f };

	Vector3 gravity{ 0, -9.81f, 0 };
	Vector3 velocity{ 0, 0, 0 };

	bool is_ground{ false };
	bool enable_collider{ true };

public:
	static float GRAVITY_MULTIPLIER;
	~CharacterController();

	void SetSizeAutomatically();
	void SetSize(float height, float radius);
	void UpdateShape();

	void SetPosition(Vector3 position);
	Vector3 GetPosition();

	void Move(Vector3 movement);
	void Jump(float power);
	void AddVelocity(Vector3 velocity);
	void SetVelocity(Vector3 velocity);

	bool IsGround() { return is_ground; }


	void CalculateVelocity();
	void ResetMovementVelocity();

	px::PxCapsuleController* GetController() { return controller; }

	float GetHeight() { return height; }
	float GetRadius() { return radius; }

	void EnableCollider();
	void DisableCollider();


public:
	virtual void Awake();
protected:
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}




public:
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Serialized(std::ofstream& ofs) override;
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Deserialized(std::ifstream& ifs) override;
};

