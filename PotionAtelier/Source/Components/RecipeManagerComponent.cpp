#include "RecipeManagerComponent.h"

std::vector<const char*> RecipeManagerComponent::potion_list_string{ "HealthPotion", "AddictionPotion", "FlamePotion", "SeductionPotion", "ManaPotion", "FrozenPotion" };
std::vector<const char*> RecipeManagerComponent::ing_list_string{ "DragonTail", "MagicFlower", "MagicWood", "SlicedDragonTail", "CompressedDragonTail", "SlicedMagicWood", "GrindedMagicWood", "GrindedMagicFlower", "CompressedMagicFlower" };


RecipeManagerComponent::RecipeManagerComponent()
{
	/*using it = IngredientType;
	using pt = PotionType;
	AddRecipe(pt::HealthPotion, { it::SlicedDragonTail, it::MagicFlower });
	AddRecipe(pt::ManaPotion, { it::CompressedMagicFlower, it::DragonTail });
	AddRecipe(pt::SeductionPotion, { it::GrindedMagicFlower, it::MagicWood });
	AddRecipe(pt::AddictionPotion, { it::SlicedMagicWood, it::MagicFlower });
	AddRecipe(pt::FlamePotion, { it::CompressedDragonTail, it::CompressedMagicFlower });
	AddRecipe(pt::FrozenPotion, { it::SlicedMagicWood, it::GrindedMagicWood });*/
}

void RecipeManagerComponent::AddRecipe(UINT result_potion_type, std::vector<IngredientType> ingredients)
{
	if (ingredients.size() != 2)
	{
		__debugbreak;
		return;
	}

	RecipeMenu menu;
	menu.potion = (PotionType)result_potion_type;
	menu.ing1 = ingredients[0];
	menu.ing2 = ingredients[1];

	menus.push_back(menu);
}

void RecipeManagerComponent::AddRecipe(UINT result_potion_type, UINT ingredients)
{
	auto ings = DecomposeIngredients(ingredients);

	AddRecipe(result_potion_type, ings);
}

void RecipeManagerComponent::SubRecipe(int index)
{
	menus.erase(menus.begin() + index);
}

PotionType RecipeManagerComponent::GetPotionType(std::vector<IngredientType> ings)
{
	if (ings.size() != 2)
	{
		return PotionType::FailurePotion;
	}

	for (auto& menu : menus)
	{
		if ((menu.ing1 == ings[0] && menu.ing2 == ings[1]) || (menu.ing1 == ings[1] && menu.ing2 == ings[0]))
		{
			return menu.potion;
		}
	}
	return PotionType::FailurePotion;
}

PotionType RecipeManagerComponent::GetPotionType(UINT ingredients)
{
	auto ings = DecomposeIngredients(ingredients);

	return GetPotionType(ings);
}

std::vector<IngredientType> RecipeManagerComponent::GetIngredients(PotionType type)
{
	for (auto& menu : menus)
	{
		if (menu.potion == type)
		{
			return { menu.ing1, menu.ing2 };
		}
	}
	return {};
}

std::vector<IngredientType> RecipeManagerComponent::DecomposeIngredients(UINT ings)
{
	std::vector<IngredientType> result;
	UINT idx = 1;
	while (idx <= static_cast<UINT>(IngredientType::Invalid))
	{
		if ((ings & idx) != 0)
		{
			result.push_back(static_cast<IngredientType>(idx));
		}
		idx <<= 1;
	}

	if (result.size() > 2)
	{
		printf("왜 재료가 2개 초과함?\n");
	}
	else if (result.size() < 2)
	{
		printf("왜 재료가 2개 미만임??\n");
		__debugbreak;
	}

	return result;
}

int RecipeManagerComponent::GetPotionTypeIndex(PotionType type)
{
	switch (type)
	{
	case PotionType::FailurePotion:
		return -1;
	case PotionType::HealthPotion:
		return 0;
	case PotionType::AddictionPotion:
		return 1;
	case PotionType::FlamePotion:
		return 2;
	case PotionType::SeductionPotion:
		return 3;
	case PotionType::ManaPotion:
		return 4;
	case PotionType::FrozenPotion:
		return 5;
	case PotionType::RainbowPotion:
		return -1;
	}

	return -1;
}

int RecipeManagerComponent::GetIngredientTypeIndex(IngredientType type)
{
	switch (type)
	{
	case IngredientType::DragonTail:
		return 0;
	case IngredientType::MagicFlower:
		return 1;
	case IngredientType::MagicWood:
		return 2;
	case IngredientType::SlicedDragonTail:
		return 3;
	case IngredientType::CompressedDragonTail:
		return 4;
	case IngredientType::SlicedMagicWood:
		return 5;
	case IngredientType::GrindedMagicWood:
		return 6;
	case IngredientType::GrindedMagicFlower:
		return 7;
	case IngredientType::CompressedMagicFlower:
		return 8;
	}

	return -1;
}

void RecipeManagerComponent::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("Recipe"))
	{
		if (ImGui::Button("Add Menu"))
		{
			AddRecipe(PotionType::HealthPotion, { IngredientType::DragonTail, IngredientType::DragonTail });
		}
		ImGui::SameLine();
		if (ImGui::Button("Sub Menu"))
		{
			if (!menus.empty())
			{
				SubRecipe(menus.size() - 1);
			}
		}


		for (int i = 0; i < menus.size(); ++i)
		{
			ImGui::Text("%d: ", i);
			ImGui::SameLine();

			int potion_type_string_idx = GetPotionTypeIndex(menus[i].potion);
			int ing_type_string_idx_1 = GetIngredientTypeIndex(menus[i].ing1);
			int ing_type_string_idx_2 = GetIngredientTypeIndex(menus[i].ing2);

			if (ImGui::Combo(std::format("Potion{}", i).c_str(), &potion_type_string_idx, potion_list_string.data(), potion_list_string.size()))
			{
				menus[i].potion = (PotionType)(1 << (potion_type_string_idx + 1));
			}

			ImGui::SameLine();

			if (ImGui::Combo(std::format("Ing{}_1", i).c_str(), &ing_type_string_idx_1, ing_list_string.data(), ing_list_string.size()))
			{
				menus[i].ing1 = (IngredientType)(1 << ing_type_string_idx_1);
			}

			ImGui::SameLine();

			if (ImGui::Combo(std::format("Ing{}_2", i).c_str(), &ing_type_string_idx_2, ing_list_string.data(), ing_list_string.size()))
			{
				menus[i].ing2 = (IngredientType)(1 << ing_type_string_idx_2);
			}
		}


		ImGui::TreePop();
	}
	ImGui::PopID();
}

void RecipeManagerComponent::Serialized(std::ofstream& ofs)
{
	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 0;
	Binary::Write::data(ofs, header); //헤더
	Binary::Write::data(ofs, version); //버전

	Binary::Write::data<size_t>(ofs, menus.size());
	Binary::Write::std_vector<RecipeMenu>(ofs, menus);
}

void RecipeManagerComponent::Deserialized(std::ifstream& ifs)
{
	size_t header = Binary::Read::data<size_t>(ifs);
	uint32_t version = 0;
	if (header != (std::numeric_limits<size_t>::max)())
	{
	}
	else
	{
		version = Binary::Read::data<uint32_t>(ifs);
	}

	size_t menus_size = Binary::Read::data<size_t>(ifs);
	menus.resize(menus_size);
	Binary::Read::std_vector<RecipeMenu>(ifs, menus);
}
