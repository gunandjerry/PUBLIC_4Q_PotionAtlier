#include "Holding.h"
#include "../ResourceFinder.h"
#include "Utility\AssimpUtility.h"
#include <ranges>

void Holding::Awake()
{
}

void Holding::Start()
{
}

void Holding::SetFocus(bool isFocus)
{
	if (current_mesh != nullptr)
	{
		auto renders =
			Utility::CollectMeshComponents(current_mesh)
			| std::views::transform([](auto& item) { return item.second; })
			| std::views::join;
		for (auto& item : renders)
		{
			item->materialAsset.customData.SetField("EdgeColorWeight", isFocus ? 1.0f : 0.0f);
		}
	}
}

bool Holding::SetType(HoldableType _type, UINT _sub_type)
{
	std::wstring tag = GetObjectTag(_type, _sub_type);

	if (tag == L"")
	{
		SetEmpty();
		return false;
	}
	else
	{
		int cn = transform.GetChildCount();
		for (int i = 0; i < cn; ++i)
		{
			GameObject& obj = transform.GetChild(i)->gameObject;
			if (obj.HasTag(tag))
			{
				if (current_mesh != nullptr) SetEmpty();
				current_mesh = &obj;
				current_mesh->Active = true;
				SetFocus(true);
				break;
			}
		}
	}

	return true;
}

void Holding::SetEmpty()
{
	if (current_mesh != nullptr)
	{
		SetFocus(false);

		current_mesh->Active = false;
		current_mesh = nullptr;
	}
}



void Holding::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("Holding Things"))
	{
		if (ImGui::Button("Set DragonTail"))
		{
			SetType(HoldableType::Ingredient, IngredientType::DragonTail);
		}
		if (ImGui::Button("Set MagicFlower"))
		{
			SetType(HoldableType::Ingredient, IngredientType::MagicFlower);
		}
		if (ImGui::Button("Set HealthPotion"))
		{
			SetType(HoldableType::Potion, PotionType::HealthPotion);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}




std::wstring Holding::GetObjectTag(HoldableType _type, UINT _sub_type)
{
	if (_type == HoldableType::Ingredient)
	{
		return ResourceFinder::GetIngredientMeshTag((IngredientType)_sub_type);
	}
	else if (_type == HoldableType::Potion)
	{
		return ResourceFinder::GetPotionMeshTag((PotionType)_sub_type);
	}

	return L"";
}
