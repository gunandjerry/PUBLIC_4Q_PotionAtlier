#pragma once
#include "framework.h"
#include "Interactable.h"
#include "HoldableTypes.h"

class EmptyTable : public Interactable
{


public:
	EmptyTable();
	virtual void Awake() {}
protected:
	virtual void Start();
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);

public:
	virtual void OnFocusIn(class PlayerController* controller) override;
	virtual void OnFocusOut(class PlayerController* controller) override;
	virtual bool OnInteract(class PlayerController* controller) override;


private:
	bool something_on{ false };
	HoldableType hold_type{ HoldableType::None };
	UINT sub_type{ 0 };
	class Holding* above_mesh{ nullptr };
	GameObject* FindHoldingObject(Transform* parent);
};

