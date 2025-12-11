#pragma once
#include "framework.h"
#include "Interactable.h"

class TrashBin : public Interactable
{
public:
	TrashBin();
	virtual void Start();
	virtual void Awake() {}
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}


public:
	virtual void OnFocusIn(class PlayerController* controller);
	virtual void OnFocusOut(class PlayerController* controller);
	virtual bool OnInteract(class PlayerController* controller);




private:
	bool on_throw{ false };
	float t{ 0.0f };
	float shrink_speed{ 3.0f };
	//float rotation_speed{ 0.0f }; // deg per sec

	//Vector3 cur_rotation{ 0, 0, 0 };

	Vector3 obj_ori_position{};
	Vector3 obj_ori_scale{};

	Vector3 goal_pos{};
	Vector3 goal_scale{};

	class Holding* obj_mesh{ nullptr };
	void FindHoldingObject(Transform* parent);
};

