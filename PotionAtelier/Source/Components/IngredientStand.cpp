#include "IngredientStand.h"
#include "PlayerController.h"
#include "Utility/SerializedUtility.h"
#include "../Object/TutorialManager.h"

std::vector<const char*> IngredientStand::imgui_items{ "DragonTail", "MagicFlower", "MagicWood" };

IngredientStand::IngredientStand()
{
	type = InteractableType::IngredientStand;
}

void IngredientStand::Awake()
{
	type = InteractableType::IngredientStand;
	ing_type = IngredientType::DragonTail;
}

void IngredientStand::OnFocusIn(PlayerController* controller)
{
	//transform.SetScale({ 1.1f, 1.1f, 1.1f });
}

void IngredientStand::OnFocusOut(PlayerController* controller)
{
	//transform.SetScale({ 1.0f, 1.0f, 1.0f });
}

bool IngredientStand::OnInteract(PlayerController* controller)
{
	if (use_stock)
	{
		if (stock == 0) return false;
	}
	if (controller->something_on_hand == true) return false;


	if (TutorialManager::GetInstance()->OnTutorial() == true)
	{
		TutorialManager::GetInstance()->ListenEvent(TutorialEventType::GET_INGREDIENT_FROM_INGREDIENT_STAND);
	}


	controller->Pick(HoldableType::Ingredient, ing_type);
	if (use_stock)
	{
		--stock;
	}

	return true;
}

void IngredientStand::Serialized(std::ofstream& ofs)
{
	Binary::Write::data<int>(ofs, (int)ing_type);
	Binary::Write::data<int>(ofs, stock);
	Binary::Write::data<bool>(ofs, use_stock);
}

void IngredientStand::Deserialized(std::ifstream& ifs)
{
	ing_type = (IngredientType)Binary::Read::data<int>(ifs);
	stock = Binary::Read::data<int>(ifs);
	use_stock = Binary::Read::data<bool>(ifs);
}

void IngredientStand::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("Ingredient Stand"))
	{
		ImGui::Checkbox("Use stock limit", &use_stock);
		if (use_stock)
		{
			ImGui::Text("Stock count: %d", stock);
		}
		
		if (ImGui::Combo("Select Ingredient", &imgui_combo_index, imgui_items.data(), imgui_items.size()))
		{
			SetIngredientType((IngredientType)(1 << imgui_combo_index));
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void IngredientStand::SetIngredientType(IngredientType type)
{
	ing_type = type;
}
