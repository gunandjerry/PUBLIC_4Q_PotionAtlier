#pragma once
#include <array>
#include <HoldableTypes.h>
#include <Components/Cooker.h>
#include "UIPoping.h"
#include "TextPoping.h"

// 배경 이미지 304 기준
// left 40 ~ center 224 ~ right 40


class HandMillCooker : public Cooker
{
	Vector2 ui_anchor{ 0, -60 };

	Vector2 ui_img_size{ 304 * 0.7f, 180 * 0.7f };
	float ui_center_width{ 265 * 0.7f };
	float ui_target_width{ 39 * 0.7f };
	float ui_target_success_tolerance{ 0.0f };
	float ui_target_success_tolerance_half{ 0.0f };
	float ui_number_scale{ 0.4f };

	UIPoping* ui_background{ nullptr };
	UIPoping* ui_target[2]{ nullptr, nullptr };
	UIPoping* ui_arrow{ nullptr };
	UIPoping* ui_number_circle{ nullptr };
	TextPoping* ui_number{ nullptr };
	Vector2 ui_number_additional_anchor{ -5, -32 };


	/* 동작 아이콘 */
	float icon_y_delta{ 60 };
	bool icon_show{ false };
	bool on_focus{ false };
	class UIPoping* icon{ nullptr };

	/* 게임 */
	int step = 0;
	int sub_step = 0;
	float t{ 0.0f };
	float arrow_speed{ 0.5f };
	bool go_right{ true };
	const float target_pos[2][2]{ {0.2f, 0.8f }, { 0.6f, 0.4f } };
	bool target_clear[2][2]{ {false,false},{false,false} };

	IngredientType ingredient_type{ IngredientType::DragonTail };
	class Holding* ingredient_graphic{ nullptr };
	GameObject* grinder_mesh{ nullptr };


	/* Audio */
	class ParticleSpawnComponent* VFX_Cut{ nullptr };
	class AudioBankClip* squeeze_sound{ nullptr };

	std::string cut{};

	class GameManagerComponent* gm{ nullptr };

public:
	HandMillCooker();
	virtual void Awake() override {}
	virtual void Start() override;
protected:
	virtual void Update() override;
	virtual void FixedUpdate() override {}
	virtual void LateUpdate() override {}

public:
	void FindGrinderMesh(Transform* parent);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;


public:
	virtual void OnFocusIn(class PlayerController* controller) override;
	virtual void OnFocusOut(class PlayerController* controller) override;
	virtual bool OnInteract(PlayerController* controller) override;
	virtual void OnCook(PlayerController* controller) override;
	void JobDone(PlayerController* controller);

	void Initialize();
	void GameStart(IngredientType type);
	bool IsInteractable(PlayerController* controller);

public:
	void SetTargetPosition();
	void SetArrowPosition();
	void InitializeTargetClearFlag();

};

