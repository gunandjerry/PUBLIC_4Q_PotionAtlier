#pragma once
#include "Interactable.h"

// Memo
// 테스트용 컴포넌트
// 지금으로선 사용하지 않음

class Throwable : public Interactable
{

	bool on_move{ false };
	float hold_time{ 0.2f };
	float distance_to_goal{ 0.0f }; // 필요하지 않음
	Vector3 current_position{ 0,0,0 };
	Vector3 goal_position{ 0, 1.0f, 1.7f };
	float t{ 0.0f };

public:
	virtual ~Throwable() = default;
	virtual void Awake();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}



public:
	virtual void OnFocusIn(class PlayerController* controller) override;
	virtual void OnFocusOut(class PlayerController* controller) override;
	virtual bool OnInteract(class PlayerController* controller) override;
	void Throw(Vector3 direction);
};

