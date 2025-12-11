#pragma once
#include <framework.h>
#include <vector>
#include "Interactable.h"
#include "../HoldableTypes.h"

class IngredientStand : public Interactable
{
	IngredientType ing_type{ IngredientType::DragonTail };
	int stock{ 10 };
	bool use_stock{ false };

	static std::vector<const char*> imgui_items;
	int imgui_combo_index{ 0 };

public:
	IngredientStand();
	virtual void Awake();
protected:
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}

public:
	virtual void OnFocusIn(class PlayerController* controller) override;
	virtual void OnFocusOut(class PlayerController* controller) override;
	virtual bool OnInteract(class PlayerController* controller) override;
	IngredientType GetIngredientType() { return ing_type; }


public:
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
	virtual void InspectorImguiDraw() override;
private:
	void SetIngredientType(IngredientType type);
};

