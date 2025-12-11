#pragma once
#include "Interactable.h"

// Memo
// 주방 기구 베이스 컴포넌트
// 상속해서 사용하세요.


class Cooker : public Interactable
{
protected:


public:
	virtual ~Cooker() = default;
	Cooker();

public:
	virtual void Awake() {}
protected:
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}

public:
	virtual void OnCook(class PlayerController* controller) abstract;

protected:
	GameObject* FindHoldingObject(Transform* parent);
};
