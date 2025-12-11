#pragma once
#include <framework.h>
#include "Interactable.h"
#include "../HoldableTypes.h"
#include "UIPoping.h"
#include <vector>
#include <map>


class Cauldron : public Interactable
{

	// 0 empty -> interact to put ing.
	// 1 ready to boil -> interact to boil
	// 2 boil -> wait certain time
	// 3 done -> interact to get potion
	// 4 overcooked -> interact to get potion
	int step = 0;
	int sub_step = 0;
	float elapsed_time{ 0.0f };
	float boiling_time{ 4.0f };
	float overcook_threshold{ 3.0f }; // n초 뒤부터 상하기? 시작
	float overcook_time{ 6.0f }; // n초째에 완전히 상함
	UINT current_ingredients{ 0 };
	PotionType result_potion_type{ PotionType::FailurePotion };

	// normal
	std::vector<std::wstring> ui_paths;
	UIPoping* ui_background_ing{ nullptr };
	float ui_background_ing_draw_speed = 0;
	UIPoping* ui_background_potion{ nullptr };
	float ui_background_potion_draw_speed = 1;
	// mask
	UIPoping* ui_mask_ing{ nullptr };
	UIPoping* ui_mask_potion{ nullptr };
	// draw on mask
	UIPoping* ui_fill{ nullptr };
	float ui_fill_draw_speed = 0;
	//UIPoping* ui_bubble_border{ nullptr };
	UIPoping* ui_divide_input{ nullptr };
	float ui_divide_input_draw_speed = 1;
	UIPoping* ing_ui{ nullptr };
	float ui_ing_draw_speed = 2;
	UIPoping* potion_ui{ nullptr };
	float ui_potion_draw_speed = 3;

	// HSV / R 0-360, S V 0-100
	Vector2 ui_bubble_fill_size{ 160, 70 };
	float ui_bubble_fill_ad_anchor_y{ -17 };
	Vector4 ui_bubble_fill_init{ 90.0f, 100.0f, 100.0f ,1 };
	Vector4 ui_bubble_fill_init_rgb{};
	Vector4 ui_bubble_fill_goal{ 15.0f, 100.0f, 100.0f, 1 };
	Vector4 ui_bubble_fill_goal_rgb{};


	/*Vector4 ui_bubble_fill_color{ 0.1f, 0.5f, 0.1f, 1.0f };
	Vector4 ui_bubble_fill_burn_color{ 1.0f, 0.0f, 0.0f, 1.0f };*/
	Vector2 ui_bubble_ing_size{ 172, 116 };
	Vector2 ui_bubble_potion_size{ 90, 116 };
	Vector2 ui_bubble_anchor{ 0, 60.0f };
	float icon_additional_anchor_y{ -20.0f };

	Vector2 overcook_warning_sign_size{ 50, 50 };
	Vector2 overcook_warning_sign_anchor{ 0, 0 };
	UIPoping* overcook_warning_sign{ nullptr };
	float overcook_warning_interval{ 0.5f };
	bool overcook_warning_show{ false };
	float overcook_warning_elapsed_time{ 0.0f };

	int feverCount = 0;
	int feverMaxCount = 2;
	// 피버 타임에 카운터로 날아가는 포션 UI
	static constexpr unsigned int fever_potion_num{ 10 };
	bool initialize_flying_potion_anchor{ false };
	bool use_flying_potion_effect{ true };
	UIPoping* fever_potion[fever_potion_num]{ nullptr, nullptr, nullptr };
	float fever_potion_t[fever_potion_num]{ 0, 0, 0 };
	bool fever_potion_use[fever_potion_num]{ false, false, false };
	Vector2 fever_potion_pos_anchor_goal{};
	float fever_potion_flew_speed{ 1.0f };



	class GameManagerComponent* gm{ nullptr };

	class AudioBankClip* boilAudio{ nullptr };
	class AudioBankClip* SFX_Fall{ nullptr };
	class AudioBankClip* warningAudio{ nullptr };
	class AudioBankClip* failAudio{ nullptr };
	class ParticleSpawnComponent* VFX_BrewSmoke{ nullptr };
	class ParticleSpawnComponent* VFX_BrewFinish{ nullptr };
	class ParticleSpawnComponent* VFX_BrewFail{ nullptr };
	class ParticleSpawnComponent* VFX_FeverBubble{ nullptr };
	class ParticleSpawnComponent* VFX_Push{ nullptr };

	std::string brew_smoke{};
	std::string brew_finish{};
	std::string brew_fail{};
	std::string fever_bubble{};
	std::string push{};

	std::vector<GameObject*> feverPotion;

	class PlayerController* player_controller{ nullptr };
	bool on_bellowing{ false };
public:
	virtual void OnFocusIn(class PlayerController* controller) override;
	virtual void OnFocusOut(class PlayerController* controller) override;
	virtual bool OnInteract(class PlayerController* controller) override;


public:
	Cauldron();
	virtual void Awake();
	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}
public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;

	void Initialize();
	void InitializeOnFever();
	void StartBoil(PlayerController* controller);
	// obsolete
	//void ReturnIngBack(PlayerController* controller);
	void Boil();
	void BoilingDone();
	void Overcooked();



	void ThrowFeverPotionToTheCounter();



	Vector4 HSVtoRGB(Vector4 HSV);
};



