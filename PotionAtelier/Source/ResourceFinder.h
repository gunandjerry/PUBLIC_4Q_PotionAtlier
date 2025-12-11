#pragma once
#include <string>
#include <vector>
#include <array>
#include "HoldableTypes.h"


#define ResourceDir(x) L"./Resource/" x

enum class PlayerAnimType
{
	None = (1 << 0),
	
	Idle = (1 << 1),
	Idle_Hold_Broom = (1 << 2),
	
	Move = (1 << 3),
	Move_Hold_Normal = (1 << 4),
	Move_Hold_Broom = (1 << 5),
	Move_Hold_Broom_Scale = (1 << 6),

	Attack_With_Broom = (1 << 7),

	Chopping = (1 << 8),
	Squeezing = (1 << 9),
	Grinding = (1 << 10),
	BlowingBellows = (1 << 11),
};

enum class GnomeAnimType
{
	None,
	Idle,
	Run,
	Hit,
	Dance
};

class ResourceFinder
{
public:
	static std::wstring GetIngredientUIPath(IngredientType type)
	{
		switch (type)
		{
		case IngredientType::DragonTail:
			return ResourceDir("Icon/Icon_Item_Tail.png");
		case IngredientType::MagicFlower:
			return ResourceDir("Icon/Icon_Item_Flower.png");
		case IngredientType::MagicWood:
			return ResourceDir("Icon/Icon_Item_Wood.png");
		case IngredientType::SlicedDragonTail:
			return ResourceDir("Icon/Item_CutTail_pot.png");
		case IngredientType::CompressedDragonTail:
			return ResourceDir("Icon/Item_SqueezeTail_pot.png");
		case IngredientType::CompressedMagicFlower:
			return ResourceDir("Icon/Item_SqueezeFlower_pot.png");
		case IngredientType::GrindedMagicFlower:
			return ResourceDir("Icon/Item_GrindFlower_pot.png");
		case IngredientType::GrindedMagicWood:
			return ResourceDir("Icon/Item_Grindwood_pot.png");
		case IngredientType::SlicedMagicWood:
			return ResourceDir("Icon/Item_CutWood_pot.png");
		}
		return L"";
	}
	static std::vector<std::wstring> GetIngredientSlicedPartsUIPaths(IngredientType type)
	{
		std::vector<std::wstring> paths;
		switch (type)
		{
		case IngredientType::DragonTail:
		{
			for (int i = 0; i < 5; ++i)
			{
				paths.push_back(std::format(ResourceDir("Textures/QTE/CuttingBoard/UI_QTE_Cut_Tail_0{}.png"), 5 - i));
			}
			return paths;
		}
		case IngredientType::MagicWood:
		{
			for (int i = 0; i < 5; ++i)
			{
				paths.push_back(std::format(ResourceDir("Textures/QTE/CuttingBoard/UI_QTE_Cut_Wood_0{}.png"), 5 - i));
			}
			return paths;
		}
		}
		return {};
	}
	static std::array<std::wstring, 4> GetSqueezeQTEUIPaths()
	{
		std::array<std::wstring, 4> paths{};
		paths[0] = ResourceDir("Sample/UI_QTE_Squeeze_BG.png");
		paths[1] = ResourceDir("Sample/UI_QTE_Squeeze_Border.png");
		paths[2] = ResourceDir("Sample/UI_QTE_Squeeze_Section.png");
		paths[3] = ResourceDir("Sample/UI_QTE_Squeeze_Fill.png");
		return paths;
	}
	static std::wstring GetPotionUIPath(PotionType type)
	{
		switch (type)
		{
		case PotionType::FailurePotion:
			return ResourceDir("Icon/Icon_Item_Potion7.png");
		case PotionType::HealthPotion:
			return ResourceDir("Icon/Icon_Item_Potion1.png");
		case PotionType::AddictionPotion:
			return ResourceDir("Icon/Icon_Item_Potion4.png");
		case PotionType::FlamePotion:
			return ResourceDir("Icon/Icon_Item_Potion3.png");
		case PotionType::SeductionPotion:
			return ResourceDir("Icon/Icon_Item_Potion5.png");
		case PotionType::ManaPotion:
			return ResourceDir("Icon/Icon_Item_Potion2.png");
		case PotionType::FrozenPotion:
			return ResourceDir("Icon/Icon_Item_Potion6.png");
		case PotionType::RainbowPotion:
			return ResourceDir("Icon/Icon_Item_Potion8.png");
		}
		return L"";
	}
	static std::wstring GetIngredientMeshTag(IngredientType type)
	{
		switch (type)
		{
		case IngredientType::DragonTail:
			return L"DragonTail";
		case IngredientType::MagicFlower:
			return L"MagicFlower";
		case IngredientType::MagicWood:
			return L"MagicWood";
		case IngredientType::SlicedDragonTail:
			return L"SlicedDragonTail";
		case IngredientType::SlicedMagicWood:
			return L"SlicedMagicWood";
		case IngredientType::CompressedDragonTail:
			return L"CompressedDragonTail";
		case IngredientType::CompressedMagicFlower:
			return L"CompressedMagicFlower";
		case IngredientType::GrindedMagicFlower:
			return L"GrindedMagicFlower";
		case IngredientType::GrindedMagicWood:
			return L"GrindedMagicWood";
		}
		return L"";
	}
	static std::wstring GetPotionMeshTag(PotionType type)
	{
		switch (type)
		{
		case PotionType::FailurePotion:
			return L"FailurePotion";
		case PotionType::HealthPotion:
			return L"HealthPotion";
		case PotionType::AddictionPotion:
			return L"AddictionPotion";
		case PotionType::FlamePotion:
			return L"FlamePotion";
		case PotionType::SeductionPotion:
			return L"SeductionPotion";
		case PotionType::ManaPotion:
			return L"ManaPotion";
		case PotionType::FrozenPotion:
			return L"FrozenPotion";
		case PotionType::RainbowPotion:
			return L"RainbowPotion";
		}
		return L"";


		/*switch (type)
		{
		case PotionType::FailedPotion:
			return L"FailedPotion";
		case PotionType::HealthPotion:
			return L"HealthPotion";
		case PotionType::AddictionPotion:
			return L"AddictionPotion";
		case PotionType::FlamePotion:
			return L"FlamePotion";
		case PotionType::SeductionPotion:
			return L"SeductionPotion";
		case PotionType::ManaPotion:
			return L"ManaPotion";
		case PotionType::FrozenPotion:
			return L"FrozenPotion";
		case PotionType::AnonymousPotion:
			return L"AnonymousPotion";
		}
		return L"";*/
	}
	static IngredientType GetSliced(IngredientType type)
	{
		switch (type)
		{
		case IngredientType::DragonTail:
			return IngredientType::SlicedDragonTail;
		case IngredientType::MagicWood:
			return IngredientType::SlicedMagicWood;
		}
		return IngredientType::Invalid;
	}
	static IngredientType GetGround(IngredientType type)
	{
		switch (type)
		{
		case IngredientType::MagicFlower:
			return IngredientType::GrindedMagicFlower;
		case IngredientType::MagicWood:
			return IngredientType::GrindedMagicWood;
		}
		return IngredientType::Invalid;
	}
	static IngredientType GetCompressed(IngredientType type)
	{
		switch (type)
		{
		case IngredientType::MagicFlower:
			return IngredientType::CompressedMagicFlower;
		case IngredientType::DragonTail:
			return IngredientType::CompressedDragonTail;
		}
		return IngredientType::Invalid;
	}


	static const wchar_t* GetPlayerAnimationClipName(PlayerAnimType type)
	{
		switch (type)
		{
		case PlayerAnimType::None:
			return L"";
		case PlayerAnimType::Idle:
			return L"rig.001|Maincharacter_hold_idle";
		case PlayerAnimType::Idle_Hold_Broom:
			return L"rig.001|Maincharacter_broomhold_idle";
		case PlayerAnimType::Move:
			return L"rig.001|Maincharacter_move";
		case PlayerAnimType::Move_Hold_Normal:
			return L"rig.001|Maincharacter_hold_move";
		case PlayerAnimType::Move_Hold_Broom:
			return L"rig.001|Maincharacter_broomhold_move";
		case PlayerAnimType::Move_Hold_Broom_Scale:
			return L"rig.001|Maincharacter_broomhold_move_scale";
		case PlayerAnimType::Attack_With_Broom:
			return L"rig.001|Maincharacter_broomhold_attack";
		case PlayerAnimType::Chopping:
			return L"rig.001|Maincharacter_chop";
		case PlayerAnimType::Squeezing:
			return L"rig.001|Maincharacter_squeez";
		case PlayerAnimType::Grinding:
			return L"rig.001|Maincharacter_grind";
		case PlayerAnimType::BlowingBellows:
			return L"rig.001|Maincharacter_bellows";
		}
		return L"";
	}

	static const wchar_t* GetGnomeAnimationClipName(GnomeAnimType type)
	{
		switch (type)
		{
		case GnomeAnimType::None:
			return L"";
		case GnomeAnimType::Idle:
			return L"rig|idle_50";
		case GnomeAnimType::Run:
			return L"rig|run_50";
		case GnomeAnimType::Hit:
			return L"rig|hit_50";
		case GnomeAnimType::Dance:
			return L"rig|dance";
		}
		return L"";
	}

	static std::wstring GetTutoButtonImage(int index)
	{
		return std::format(ResourceDir("UI/Tutorial/Img_Tutorial_0{}.png"), index);
	}
	static std::vector<std::wstring> GetCauldronBubbleImages()
	{
		std::vector<std::wstring> paths;

		// 재료
		paths.push_back(ResourceDir("UI/Cauldron/UI_PotBubble1.png"));
		paths.push_back(ResourceDir("UI/Cauldron/UI_PotBubble2.png"));
		paths.push_back(ResourceDir("UI/Cauldron/UI_Bubble_Fill_White.png"));
		paths.push_back(ResourceDir("UI/Cauldron/UI_PotBubble4.png"));
		paths.push_back(ResourceDir("UI/Cauldron/UI_PotBubble4-2.png"));
		paths.push_back(ResourceDir("UI/Cauldron/UI_PotBubble4-3.png"));

		// 포션
		paths.push_back(ResourceDir("UI/Cauldron/UI_PotBubble1-1.png"));
		paths.push_back(ResourceDir("UI/Cauldron/UI_PotBubble2-2.png"));

		return paths;
	}
	static std::wstring GetTextBubbleImage(bool for_player)
	{
		if (for_player)
		{
			return ResourceDir("Textures/UI_TalkBubble.png");
		}
		else
		{
			return ResourceDir("Textures/UI_TalkBubble_Customer.png");
		}
	}
	static std::wstring GetTutorialBubbleImage()
	{
		return ResourceDir("UI/Tutorial/UI_PopUp.png");
	}
	static std::wstring GetTutorialPopupImage(int index)
	{
		if (index == 0)
		{
			return ResourceDir("UI/Tutorial/Img_Tutorial_01.png");
		}
		else if (index == 1)
		{
			return ResourceDir("UI/Tutorial/Img_Tutorial_02.png");
		}
		else if (index == 2)
		{
			return ResourceDir("UI/Tutorial/Img_Tutorial_03.png");
		}
		else if (index == 3)
		{
			return ResourceDir("UI/Tutorial/Img_Tutorial_04.png");
		}
		else if (index == 4)
		{
			return ResourceDir("UI/Tutorial/Img_Tutorial_05.png");
		}
		else if (index == 5)
		{
			return ResourceDir("UI/Tutorial/UI_Tutorial_Left.png");
		}
		else if (index == 6)
		{
			return ResourceDir("UI/Tutorial/UI_Tutorial_Right.png");
		}
	}
	static std::wstring GetOvercookWarningSign()
	{
		return ResourceDir("Icon/IconUI_Warning.png");
	}
	static std::vector<std::wstring> GetStaminaUI()
	{
		std::vector<std::wstring> paths;
		paths.push_back(ResourceDir("UI/Stamina/UI_Patience_Bar_Fill.png"));
		paths.push_back(ResourceDir("UI/Stamina/UI_Stamina_Fill.png"));
		/*paths.push_back(ResourceDir("UI/Stamina/UI_Stamina_Fill.png"));
		paths.push_back(ResourceDir("UI/Stamina/UI_Patience_Bar_Border.png"));*/

		return paths;
	}
};

