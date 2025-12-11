#pragma once
#include <framework.h>
#include <vector>
#include <map>
#include "../HoldableTypes.h"

struct RecipeMenu
{
	PotionType potion{ PotionType::FailurePotion };
	IngredientType ing1{ IngredientType::Invalid };
	IngredientType ing2{ IngredientType::Invalid };
};

class RecipeManagerComponent : public Component
{
	std::vector<RecipeMenu> menus;
	//std::map<UINT, UINT> recipe;
	//std::map<PotionType, std::vector<IngredientType>> reverse;

	static std::vector<const char*> potion_list_string;
	static std::vector<const char*> ing_list_string;

public:
	static const std::vector<const char*>& GetPotionListString() { return potion_list_string; }
	static int GetPotionTypeIndex(PotionType type);

	RecipeManagerComponent();

	void AddRecipe(UINT result_potion_type, std::vector<IngredientType> ingredients);
	void AddRecipe(UINT result_potion_type, UINT ingredients);
	void SubRecipe(int index);

	PotionType GetPotionType(std::vector<IngredientType> ingredients);
	PotionType GetPotionType(UINT ingredients);

	std::vector<IngredientType> GetIngredients(PotionType type);

private:
	std::vector<IngredientType> DecomposeIngredients(UINT ings);
	int GetIngredientTypeIndex(IngredientType type);



public:
	virtual void Awake() {}
	virtual void Start() {}
protected:
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
};

