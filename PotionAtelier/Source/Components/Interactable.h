#pragma once
#include "framework.h"

// Memo
// Space바로 상호 작용 가능한 객체 베이스 컴포넌트
// 플레이어 Sweep에 걸린 첫 번째 오브젝트 -> OnFocusIn
// 벗어남 -> OnFocusOut
// Sweep에 걸린 상태로 상호작용키 -> OnInteract

enum class InteractableType
{
	None,
	Cooker_Cut,
	Cooker_Grind,
	Cooker_Squeeze,
	Counter, // 납품 창구
	IngredientStand, // 원재료 수납장
	TrashBin, // 쓰레기통
	EmptyTable, // 물건 올려놓는 놈
	Cauldron // 가마솥
};

class Interactable : public Component
{
public:
	InteractableType type{ InteractableType::None };

	virtual void Awake() {}
protected:
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}



public:
	virtual void OnFocusIn(class PlayerController* controller) {}
	virtual void OnFocusOut(class PlayerController* controller) {}
	virtual bool OnInteract(class PlayerController* controller) { return true; }

	virtual void OnAttacked(class PlayerController* controller) {}
};