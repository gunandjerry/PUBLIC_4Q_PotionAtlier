#pragma once
#include "framework.h"

class BroomComponent : public Component
{
	//Vector3 init_position{};
	//Vector3 init_rotation{};
	//Vector3 init_scale{};

	//// on hold
	//Vector3 hold_coord{ -0.837f, 0.607f, -1.254f };
	//Vector3 hold_rotation{ 62.67f, 153.674f, 126.924f };
	//Vector3 hold_scale{ 0.25f, 0.25f, 0.25f };

	bool on_hold{ false };

public:
	virtual void Start();
	virtual void Awake() {}
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}

public:
	//virtual void OnFocusIn(class PlayerController* controller);
	//virtual void OnFocusOut(class PlayerController* controller);
	//virtual bool OnInteract(class PlayerController* controller);

private:
	//void PutDown(class PlayerController* controller);
	//void Hold(class PlayerController* controller);

	//void LockRotation();
	//void UnlockRotation();
};

