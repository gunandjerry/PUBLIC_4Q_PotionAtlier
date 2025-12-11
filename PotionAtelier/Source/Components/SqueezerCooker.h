#pragma once
#include <array>
#include <HoldableTypes.h>
#include <Components/Cooker.h>

class PlayerController;

struct JJuckkk
{
	//
	std::array<class UIRenderComponenet*, 4> renderer{};
};

class SqueezerCooker : public Cooker
{
public:
	SqueezerCooker();

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void InspectorImguiDraw() override;

protected:
	virtual void Update() override;

	virtual void FixedUpdate() {}
	virtual void LateUpdate() {}

public:
	virtual void OnFocusIn(PlayerController* controller) override;
	virtual void OnFocusOut(PlayerController* controller) override;
	virtual bool OnInteract(PlayerController* controller) override;

	virtual void OnCook(PlayerController* controller) override;

private:
	bool IsInteractable(PlayerController* controller);
	void GameStart(IngredientType type);
	void SetSuccessZone();
	void ProcessAnimation();
	void UpdatePosition();
	void Initialize();
	void Failed();

	void SaveVFX();
	void LoadVFX();

private:
	JJuckkk jjuckkk;
	IngredientType ingredient_type{ IngredientType::DragonTail };
	class PlayerController* cached_controller{ nullptr };
	class Holding* ingredient_graphic{ nullptr };
	class ParticleSpawnComponent* VFX_Cut{ nullptr };
	class AudioBankClip* squeeze_sound{ nullptr };

	inline static constexpr const wchar_t* VFX_CutDataPath = L"Resource/VFX/VFX_Cut.BinaryData";
	std::string cut{};

	unsigned int step{ 0 };

	Vector2 anchor{ 0, 0 };
	int jjuckk_offset_height = 60;
	float section_range_min = 20.f;
	float section_range_max = 30.f;
	float section_offset_min = 40.f;
	float section_offset_max = 60.f;
	float section_range = 0.f;
	float section_x = 0.f;
	float fill_speed = 0.75f;
	float current_gauge = 0.0f;

	float bar_width = 100.f;
	float bar_height = 40.f;

	float icon_y_delta{ 60 };
	bool icon_show{ false };
	bool on_focus{ false };
	bool is_cook{ false };
	class UIPoping* icon{ nullptr };

	bool squeeze_success{ false };
};
