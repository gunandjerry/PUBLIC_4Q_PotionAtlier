#pragma once
#include "framework.h"
#include "Cooker.h"
#include "../HoldableTypes.h"

// Memo
// Funcking 만삣쀠
// 이딴걸로 FSM 쓰지마
// Step으로 Enough

struct NyamNyam
{
	// client coordinate
	Vector2 original_position{ 0, 0 };
	Vector2 goal_position{ 0, 0 };
	float original_rotation{ 0 };
	float goal_rotation{ 0 };

	float t{ 0 };
	bool is_cut{ false };
	class UIRenderComponenet* renderer{ nullptr };
};

class CuttingBoardCooker : public Cooker
{
	unsigned int step{ 0 };
	unsigned int sub_step{ 0 };

	Vector2 NyamNyamAnchor{ 0, 0 };
	Vector2 image_anchor{ 0, 40 };
	float part_gap_px{ 29 };
	float part_width_px{ 40 };
	float part_height_px{ 40 };
	float part_size_mult{ 1.0f };

	// 위에 올려진 메쉬 나타나고 사라질 때 스케일 애님
	float scale_anim_t{ 0.0f };
	float scale_anim_speed = 16.0f;

	float goal_gap{ part_gap_px };
	float goal_width{ part_width_px };
	float goal_height{ part_height_px };

	bool disappearing{ false };

	Vector2 dragon_tail_add_pos[5]{ {0, 0}, };
	Vector2 magic_wood_add_pos[5]{ {0, 0}, };
	
	// 잘랐을 때 얼마나 날아감?
	Vector2 flew_distance{ 14, 8 };
	float flew_speed{ 20.0f };
	float flew_rotation{ 30.0f };

	const unsigned int divide_num{ 5 };
	std::vector<NyamNyam> nyams;

	class GameManagerComponent* gm{ nullptr };


	// Focus 잡혔을 때 아이콘
	float icon_y_delta{ 60 };
	bool icon_show{ false };
	bool on_focus{ false };
	class UIPoping* icon{ nullptr };



	IngredientType ingredient_type{ IngredientType::DragonTail };
	class Holding* ingredient_graphic{ nullptr };
	AudioBankClip* cut_sound{ nullptr };
	class ParticleSpawnComponent* VFX_Cut{ nullptr };
	std::string cut{};

public:
	CuttingBoardCooker();

public:
	virtual void Awake();
	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}

public:
	virtual void OnFocusIn(class PlayerController* controller) override;
	virtual void OnFocusOut(class PlayerController* controller) override;
	virtual bool OnInteract(class PlayerController* controller) override;

private:
	void InputTexture(IngredientType type);
	void Initialize();

	void UpdatePartsBasePosition();
	void SetPartsPosition();

	void ProcessAnimation();

public:
	void GameStart(IngredientType type);
	virtual void OnCook(class PlayerController* controller) override;

private:
	bool IsInteractable(class PlayerController* controller);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
};
