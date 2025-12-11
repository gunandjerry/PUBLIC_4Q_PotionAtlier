#include "Cauldron.h"
#include "PlayerController.h"
#include "../ResourceFinder.h"
#include "../Object/TutorialManager.h"
#include <Object\AudioPlayerObject.h>
#include "Object/RecipeManager.h"
#include "Object/GameManager.h"

using namespace TimeSystem;


void Cauldron::OnFocusIn(PlayerController* controller)
{
	//transform.SetScale({ 1.1f, 1.1f, 1.1f });
}

void Cauldron::OnFocusOut(PlayerController* controller)
{
	//transform.SetScale({ 1.0f, 1.0f, 1.0f });
	on_bellowing = false;
}

bool Cauldron::OnInteract(PlayerController* controller)
{
	if (gm->IsFever())
	{
		if (step < 9)
		{
			InitializeOnFever();
			step = 9;

		}
	}
	else
	{
		if (step >= 9) step = 0;
	}


	if (step == 0)
	{
		// 빈 손일 때 상호작용하면 다시 꺼내기? -> ㄴㄴ
		if (controller->something_on_hand == false) return false;
		if (controller->hold_type != HoldableType::Ingredient) return false;

		// tutorial
		TutorialManagerComponent* tm = TutorialManager::GetInstance();
		if (tm->OnTutorial() == true) tm->ListenEvent(TutorialEventType::INPUT_INGREDIENT_INTO_CAULDRON);

		StartBoil(controller);

		++sub_step;

		if (sub_step == 2)
		{
			++step;
			// 재료 두 개 넣자마자 자동 끓이기
			Boil();
			++step;
		}

		return true;
	}
	else if (step == 1)
	{
		// 재료 두 개 넣자마자 자동 끓이기
		/*Boil();
		++step;*/

		return true;
	}
	else if (step == 2)
	{

		return true;
	}
	else if (step == 3 || step == 4)
	{
		if (controller->something_on_hand == true) return false;

		TutorialManagerComponent* tm = TutorialManager::GetInstance();
		if (tm->OnTutorial() == true) tm->ListenEvent(TutorialEventType::GET_POTION_FROM_CAULDRON);

		controller->Pick(HoldableType::Potion, result_potion_type);
		if (VFX_BrewFinish) VFX_BrewFinish->isSpawnParticlesByTime = false;
		if (VFX_BrewFail) VFX_BrewFail->isSpawnParticlesByTime = false;
		Initialize();

		return true;
	}
	else if (step == 9 /* FEVER */)
	{
		if (controller->something_on_hand == true) return false;

		on_bellowing = true;
		player_controller = controller;
		controller->PlayCookingAnimation(type);

		if (++feverCount >= feverMaxCount)
		{
			ui_background_potion->SmoothAppear(0);
			ui_mask_potion->SmoothAppear(0);
			ui_fill->bubbles[0]->renderer->SetColor(ui_bubble_fill_init_rgb);
			potion_ui->SmoothAppear(0);
			potion_ui->SetTexture(0, ResourceFinder::GetPotionUIPath(PotionType::RainbowPotion));
			if (VFX_FeverBubble) VFX_FeverBubble->isSpawnParticlesByTime = true;
			if (VFX_Push) VFX_Push->isSpawnParticlesByTime = true;
			step = 10;
			feverCount = 0;
		}
	}
	else if (step == 10 /* FEVER */)
	{
		if (controller->something_on_hand == true) return false;

		GameManager::GetGM().ServeOrder(PotionType::RainbowPotion);
		GameManager::GetGM().AddScore(PotionType::RainbowPotion, 0);

		if (use_flying_potion_effect == true)
		{
			ThrowFeverPotionToTheCounter();
		}

		// controller->InformQTEDone();
		if (VFX_FeverBubble) VFX_FeverBubble->isSpawnParticlesByTime = false;
		if (VFX_Push) VFX_Push->isSpawnParticlesByTime = false;
		if (VFX_BrewFinish) VFX_BrewFinish->isSpawnParticlesByTime = false;
		if (VFX_BrewFail) VFX_BrewFail->isSpawnParticlesByTime = false;

		InitializeOnFever();
		step = 9;

		return true;
	}
}

Cauldron::Cauldron()
{
	type = InteractableType::Cauldron;
}

void Cauldron::Awake()
{
}

void Cauldron::Start()
{
	ui_paths = std::move(ResourceFinder::GetCauldronBubbleImages());

	ui_background_ing = &AddComponent<UIPoping>();
	ui_background_ing->Initialize(1, 0, ui_bubble_ing_size, { ui_bubble_anchor.x, -ui_bubble_anchor.y });
	ui_background_ing->SetTexture(0, ui_paths[0]);
	ui_background_ing->bubbles[0]->renderer->drawSpeed = ui_background_ing_draw_speed;

	ui_background_potion = &AddComponent<UIPoping>();
	ui_background_potion->Initialize(1, 0, ui_bubble_potion_size, {ui_bubble_anchor.x, -ui_bubble_anchor.y});
	ui_background_potion->SetTexture(0, ui_paths[6]);
	ui_background_potion->bubbles[0]->renderer->drawSpeed = ui_background_potion_draw_speed;

	ui_mask_ing = &AddComponent<UIPoping>();
	ui_mask_ing->Initialize(1, 0, ui_bubble_ing_size, { ui_bubble_anchor.x, -ui_bubble_anchor.y });
	ui_mask_ing->SetTexture(0, ui_paths[1]);
	ui_mask_ing->bubbles[0]->renderer->this_is_mask = true;

	ui_mask_potion = &AddComponent<UIPoping>();
	ui_mask_potion->Initialize(1, 0, ui_bubble_potion_size, { ui_bubble_anchor.x, -ui_bubble_anchor.y });
	ui_mask_potion->SetTexture(0, ui_paths[7]);
	ui_mask_potion->bubbles[0]->renderer->this_is_mask = true;

	ui_fill = &AddComponent<UIPoping>();
	ui_fill->Initialize(1, 0, ui_bubble_fill_size, { ui_bubble_anchor.x, -ui_bubble_anchor.y + ui_bubble_fill_ad_anchor_y });
	ui_fill->SetTexture(0, ui_paths[2]);
	ui_fill->bubbles[0]->renderer->draw_after_masking = true;
	ui_fill->bubbles[0]->renderer->drawSpeed = ui_fill_draw_speed;
	ui_fill->bubbles[0]->renderer->SetColor(ui_bubble_fill_init_rgb);
	ui_fill->bubble_size.y = 0;
	ui_fill->bubbles[0]->target_size.y = 0;
	ui_fill->bubbles[0]->cur_size.y = 0;

	/*ui_bubble_border = &AddComponent<UIPoping>();
	ui_bubble_border->Initialize(1, 0, ui_bubble_size, { ui_bubble_anchor.x, -ui_bubble_anchor.y });
	ui_bubble_border->SetTexture(0, imgs[2]);
	ui_bubble_border->bubbles[0]->renderer->draw_after_masking = true;
	ui_bubble_border->bubbles[0]->renderer->drawSpeed = 2;*/

	ui_divide_input = &AddComponent<UIPoping>();
	ui_divide_input->Initialize(1, 0, ui_bubble_ing_size, { ui_bubble_anchor.x, -ui_bubble_anchor.y });
	ui_divide_input->SetTexture(0, ui_paths[4]);
	ui_divide_input->bubbles[0]->renderer->draw_after_masking = true;
	ui_divide_input->bubbles[0]->renderer->drawSpeed = ui_divide_input_draw_speed;
	
	ing_ui = &AddComponent<UIPoping>();
	ing_ui->Initialize(2, 80, { 40, 40 }, { ui_bubble_anchor.x, -ui_bubble_anchor.y + icon_additional_anchor_y });
	ing_ui->bubbles[0]->renderer->draw_after_masking = true;
	ing_ui->bubbles[1]->renderer->draw_after_masking = true;
	ing_ui->bubbles[0]->renderer->drawSpeed = ui_ing_draw_speed;
	ing_ui->bubbles[1]->renderer->drawSpeed = ui_ing_draw_speed;

	potion_ui = &AddComponent<UIPoping>();
	potion_ui->Initialize(1, 0, { 40, 40 }, { ui_bubble_anchor.x, -ui_bubble_anchor.y + icon_additional_anchor_y });
	potion_ui->bubbles[0]->renderer->draw_after_masking = true;
	potion_ui->bubbles[0]->renderer->drawSpeed = ui_potion_draw_speed;

	overcook_warning_sign = &AddComponent<UIPoping>();
	overcook_warning_sign->Initialize(1, 0, overcook_warning_sign_size, overcook_warning_sign_anchor);
	overcook_warning_sign->SetTexture(0, ResourceFinder::GetOvercookWarningSign());

	

	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Boil"))
	{
		boilAudio = findItem->IsComponent<AudioBankClip>();
	}
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Warning"))
	{
		warningAudio = findItem->IsComponent<AudioBankClip>();
	}
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Potion_Fail"))
	{
		failAudio = findItem->IsComponent<AudioBankClip>();
	}
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Fall"))
	{
		SFX_Fall = findItem->IsComponent<AudioBankClip>();
	}
	std::wstring wstr = std::wstring(brew_smoke.begin(), brew_smoke.end());
 	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_BrewSmoke = findItem->IsComponent<ParticleSpawnComponent>();
	}
	wstr = std::wstring(brew_finish.begin(), brew_finish.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_BrewFinish = findItem->IsComponent<ParticleSpawnComponent>();
	}
	wstr = std::wstring(brew_fail.begin(), brew_fail.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_BrewFail = findItem->IsComponent<ParticleSpawnComponent>();
	}
	wstr = std::wstring(fever_bubble.begin(), fever_bubble.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_FeverBubble = findItem->IsComponent<ParticleSpawnComponent>();
	}
	wstr = std::wstring(push.begin(), push.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_Push = findItem->IsComponent<ParticleSpawnComponent>();
	}

	gm = &GameManager::GetGM();


	ui_bubble_fill_init_rgb = HSVtoRGB(ui_bubble_fill_init);
	ui_bubble_fill_goal_rgb = HSVtoRGB(ui_bubble_fill_goal);


	if (use_flying_potion_effect == true)
	{
		for (int i = 0; i < fever_potion_num; ++i)
		{
			fever_potion[i] = &AddComponent<UIPoping>();
			fever_potion[i]->Initialize(1, 0, { 48, 64 }, { 0, 0 });
			fever_potion[i]->SetTexture(0, L"./Resource/Icon/Icon_Item_Potion8.png");
		}
	}
}

void Cauldron::Update()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay()) return;
#endif
	if (gm->IsTimeEnd())
	{
		ui_background_ing->InstantDisappear(0);
		ui_background_potion->InstantDisappear(0);
		ui_mask_ing->InstantDisappear(0);
		ui_mask_potion->InstantDisappear(0);
		ui_fill->InstantDisappear(0);
		ui_divide_input->InstantDisappear(0);
		ing_ui->InstantDisappear(0);
		ing_ui->InstantDisappear(1);
		potion_ui->InstantDisappear(0);
		overcook_warning_sign->SmoothDisappear(0);
	}
	if (on_bellowing == true && gm->IsFever() == false)
	{
		on_bellowing = false;
		if (player_controller != nullptr) player_controller->InformQTEDone();
	}
	if ((step == 9 || step == 10) && gm->IsFever() == false)
	{
		Initialize();
	}


	if (step == 2)
	{
		elapsed_time += Time.DeltaTime;

		//ui_fill->anchor.y = -ui_bubble_anchor.y + (ui_bubble_ing_size.y - ui_bubble_ing_size.y * (elapsed_time / boiling_time));

		float pos_ratio = elapsed_time / boiling_time;

		UINT ori_size = (UINT)ui_bubble_fill_size.y;
		UINT cover_size = (UINT)(ori_size * pos_ratio);

		int gap = ori_size - cover_size;

		if (gap % 2 == 1) gap += 1;
		ui_fill->anchor.y = -ui_bubble_anchor.y + ui_bubble_fill_ad_anchor_y + (gap / 2);
		ui_fill->bubble_size.y = cover_size;
		ui_fill->bubbles[0]->target_size.y = cover_size;
		ui_fill->bubbles[0]->cur_size.y = cover_size;
		ui_fill->bubbles[0]->do_not_turn_off_when_scale_goto_zero = true;

		if (elapsed_time >= boiling_time)
		{
			/*ui_fill->anchor.y = -ui_bubble_anchor.y;*/
			ui_fill->anchor.y = -ui_bubble_anchor.y + ui_bubble_fill_ad_anchor_y;
			ui_fill->bubble_size.y = ori_size;
			ui_fill->bubbles[0]->target_size.y = ori_size;
			ui_fill->bubbles[0]->cur_size.y = ori_size;

			elapsed_time = 0.0f;
			BoilingDone();
			++step;
		}
	}
	else if (step == 3)
	{
		// 튜토리얼 중엔 Overcooked 기능 끄기
		if (TutorialManager::GetInstance()->OnTutorial() == true) return;

		elapsed_time += Time.DeltaTime;

		if (elapsed_time >= overcook_threshold)
		{
			if (warningAudio && !warningAudio->IsPlaying()) warningAudio->Play();
			// 경고 깜빡
			overcook_warning_elapsed_time += Time.DeltaTime;
			if (overcook_warning_elapsed_time >= overcook_warning_interval)
			{
				overcook_warning_elapsed_time -= overcook_warning_interval;
				if (overcook_warning_show == true)
				{
					overcook_warning_sign->InstantDisappear(0);
					overcook_warning_show = false;
				}
				else if (overcook_warning_show == false)
				{
					overcook_warning_sign->InstantAppear(0);
					overcook_warning_show = true;
				}
			}

			// 색상 진해지기
			Vector4 lerped_hsv = Vector4::Lerp(ui_bubble_fill_init, ui_bubble_fill_goal, (elapsed_time - overcook_threshold) / overcook_time);
			Vector4 rgb = HSVtoRGB(lerped_hsv);
			ui_fill->bubbles[0]->renderer->SetColor(rgb);

			if ((elapsed_time - overcook_threshold) >= overcook_time)
			{
				ui_fill->bubbles[0]->renderer->SetColor(ui_bubble_fill_goal_rgb);
				Overcooked();
				++step;
			}
		}
	}



	for (int i = 0; i < fever_potion_num; ++i)
	{
		if (fever_potion_use[i] == true)
		{
			fever_potion_t[i] += Time.DeltaTime * fever_potion_flew_speed;
			if (fever_potion_t[i] < 0.95f)
			{
				fever_potion[i]->anchor = fever_potion_pos_anchor_goal * fever_potion_t[i];
			}
			else if (fever_potion_t[i] >= 0.95f)
			{
				fever_potion[i]->anchor = fever_potion_pos_anchor_goal * fever_potion_t[i];
				fever_potion[i]->SmoothDisappear(0);
			}

			if (fever_potion_t[i] >= 1.0f)
			{
				fever_potion_use[i] = false;
			}
		}
	}
}

void Cauldron::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("Cauldron"))
	{
		ImGui::DragFloat("Boiling Time", &boiling_time);
		ImGui::DragFloat("Overcook Threshold", &overcook_threshold);
		ImGui::DragFloat("Overcook Time", &overcook_time);

		ImGui::Dummy({ 0, 10 });

		//ImGui::DragFloat4("UI Normal Fill Color", &ui_bubble_fill_init.x);
		//ImGui::DragFloat4("UI Overcook Fill Color", &ui_bubble_fill_goal.x);
		ImGui::DragFloat2("UI Size", &ui_bubble_ing_size.x);
		ImGui::DragFloat2("UI Anchor", &ui_bubble_anchor.x);

		ImGui::Dummy({ 0, 10 });

		ImGui::DragFloat2("Warning Sign size", &overcook_warning_sign_size.x);
		ImGui::DragFloat2("Warning Sign Anchor", &overcook_warning_sign_anchor.x);
		ImGui::DragFloat("Warning Sign Flip Interval", &overcook_warning_interval);

		ImGui::TreePop();
	}

	ImGui::InputText("VFX_BrewSmoke", (char*)brew_smoke.c_str(), brew_smoke.size(), ImGuiInputTextFlags_CallbackResize,
					 [](ImGuiInputTextCallbackData* data)
					 {
						 if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
						 {
							 std::string* str = (std::string*)data->UserData;
							 IM_ASSERT(data->Buf == str->c_str());
							 str->resize(data->BufTextLen);
							 data->Buf = str->data();
						 }
						 return 0;
					 }, &brew_smoke);

	ImGui::InputText("VFX_BrewFinish", (char*)brew_finish.c_str(), brew_finish.size(), ImGuiInputTextFlags_CallbackResize,
					 [](ImGuiInputTextCallbackData* data)
					 {
					 	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
					 	{
					 		std::string* str = (std::string*)data->UserData;
					 		IM_ASSERT(data->Buf == str->c_str());
					 		str->resize(data->BufTextLen);
					 		data->Buf = str->data();
					 	}
					 	return 0;
					 }, &brew_finish);

	ImGui::InputText("VFX_BrewFail", (char*)brew_fail.c_str(), brew_fail.size(), ImGuiInputTextFlags_CallbackResize,
					 [](ImGuiInputTextCallbackData* data)
					 {
					 	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
					 	{
					 		std::string* str = (std::string*)data->UserData;
					 		IM_ASSERT(data->Buf == str->c_str());
					 		str->resize(data->BufTextLen);
					 		data->Buf = str->data();
					 	}
					 	return 0;
					 }, &brew_fail);

	ImGui::InputText("VFX_FeverBubble", (char*)fever_bubble.c_str(), fever_bubble.size(), ImGuiInputTextFlags_CallbackResize,
		[](ImGuiInputTextCallbackData* data)
		{
			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
			{
				std::string* str = (std::string*)data->UserData;
				IM_ASSERT(data->Buf == str->c_str());
				str->resize(data->BufTextLen);
				data->Buf = str->data();
			}
			return 0;
		}, & fever_bubble);

	ImGui::InputText("VFX_Push", (char*)push.c_str(), push.size(), ImGuiInputTextFlags_CallbackResize,
		[](ImGuiInputTextCallbackData* data)
		{
			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
			{
				std::string* str = (std::string*)data->UserData;
				IM_ASSERT(data->Buf == str->c_str());
				str->resize(data->BufTextLen);
				data->Buf = str->data();
			}
			return 0;
		}, & push);

	ImGui::PopID();
}

void Cauldron::Serialized(std::ofstream& ofs)
{
	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 4;
	Binary::Write::data(ofs, header);
	Binary::Write::data(ofs, version);

	Binary::Write::data(ofs, boiling_time);
	Binary::Write::data(ofs, overcook_threshold);
	Binary::Write::data(ofs, overcook_time);
	
	Vector4 shit_fill_init{};
	Vector4 shit_fill_goal{};
	Binary::Write::Vector4(ofs, shit_fill_init);
	Binary::Write::Vector4(ofs, shit_fill_goal);
	Binary::Write::Vector2(ofs, ui_bubble_ing_size);
	Binary::Write::Vector2(ofs, ui_bubble_anchor);

	Binary::Write::Vector2(ofs, overcook_warning_sign_size);
	Binary::Write::Vector2(ofs, overcook_warning_sign_anchor);
	Binary::Write::data(ofs, overcook_warning_interval);

	if(version >= 1)
		Binary::Write::wstring(ofs, L"");

	if (version >= 2)
		Binary::Write::string(ofs, "");

	if (version >= 3)
	{
		Binary::Write::string(ofs, brew_smoke);
		Binary::Write::string(ofs, brew_finish);
		Binary::Write::string(ofs, brew_fail);
	}
	
	if (version >= 4)
	{
		Binary::Write::string(ofs, fever_bubble);
		Binary::Write::string(ofs, push);
	}
}
void Cauldron::Deserialized(std::ifstream& ifs)
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

	boiling_time = Binary::Read::data<float>(ifs);
	overcook_threshold = Binary::Read::data<float>(ifs);
	overcook_time = Binary::Read::data<float>(ifs);

	Vector4 shit_fill_init = Binary::Read::Vector4(ifs);
	Vector4 shit_fill_goal = Binary::Read::Vector4(ifs);
	//ui_bubble_fill_init = { shit_fill_init.x, shit_fill_init.y, shit_fill_init.z };
	//ui_bubble_fill_goal = { shit_fill_goal.x, shit_fill_goal.y, shit_fill_goal.z };

	ui_bubble_ing_size = Binary::Read::Vector2(ifs);
	ui_bubble_anchor = Binary::Read::Vector2(ifs);

	overcook_warning_sign_size = Binary::Read::Vector2(ifs);
	overcook_warning_sign_anchor = Binary::Read::Vector2(ifs);
	overcook_warning_interval = Binary::Read::data<float>(ifs);

	if (version >= 1)
		Binary::Read::wstring(ifs);
	if (version >= 2)
		Binary::Read::string(ifs);
	if (version >= 3)
	{
		brew_smoke = Binary::Read::string(ifs);
		brew_finish = Binary::Read::string(ifs);
		brew_fail = Binary::Read::string(ifs);
	}
	if (version >= 4)
	{
		fever_bubble = Binary::Read::string(ifs);
		push = Binary::Read::string(ifs);
	}
}

void Cauldron::Initialize()
{
	step = 0;
	sub_step = 0;
	elapsed_time = 0.0f;
	current_ingredients = 0;

	for (int i = 0; i < 2; ++i)
	{
		ing_ui->SmoothDisappear(i);
	}
	potion_ui->SmoothDisappear(0);

	ui_background_ing->SmoothDisappear(0);
	ui_background_potion->SmoothDisappear(0);
	ui_mask_ing->SmoothDisappear(0);
	ui_mask_potion->SmoothDisappear(0);
	ui_fill->SmoothDisappear(0);
	ui_divide_input->SetTexture(0, ui_paths[4]);
	ui_divide_input->SmoothDisappear(0);
	overcook_warning_sign->InstantDisappear(0);

	if (boilAudio) boilAudio->Stop();
	if (warningAudio) warningAudio->Stop();
	if (failAudio) failAudio->Stop();
}

void Cauldron::InitializeOnFever()
{
	step = 0;
	sub_step = 0;
	elapsed_time = 0.0f;
	current_ingredients = 0;

	for (int i = 0; i < 2; ++i)
	{
		ing_ui->SmoothDisappear(i);
	}
	potion_ui->SmoothDisappear(0);

	ui_background_ing->SmoothDisappear(0);
	ui_background_potion->SmoothDisappear(0);
	ui_mask_ing->SmoothDisappear(0);
	ui_mask_potion->SmoothDisappear(0);
	ui_fill->SmoothDisappear(0);
	ui_divide_input->SetTexture(0, ui_paths[4]);
	ui_divide_input->SmoothDisappear(0);
	overcook_warning_sign->InstantDisappear(0);

	if (boilAudio) boilAudio->Stop();
	if (warningAudio) warningAudio->Stop();
	if (failAudio) failAudio->Stop();
}

void Cauldron::StartBoil(PlayerController* controller)
{
	IngredientType input_ing_type = (IngredientType)controller->hold_subtype;
	current_ingredients |= controller->hold_subtype;
	controller->PutDown();

	std::wstring path = ResourceFinder::GetIngredientUIPath(input_ing_type);

	ing_ui->SetTexture(sub_step, path);
	ing_ui->SmoothAppear(sub_step);
	if (SFX_Fall) SFX_Fall->Play();
	if (sub_step == 0)
	{
		ui_background_ing->SmoothAppear(0);
		ui_mask_ing->SmoothAppear(0);
		ui_divide_input->SmoothAppear(0);
		ui_fill->InstantAppear(0);
 		ui_fill->bubbles[0]->renderer->SetColor(ui_bubble_fill_init_rgb);

		ui_fill->bubble_size.y = 0;
		ui_fill->bubbles[0]->target_size.y = 0;
		ui_fill->bubbles[0]->cur_size.y = 0;
	}
}

// obsolete
//void Cauldron::ReturnIngBack(PlayerController* controller)
//{
//	if (controller->something_on_hand == true) return;
//	if (step != 0 || sub_step != 1) return;
//
//	controller->Pick(HoldableType::Ingredient, current_ingredients);
//
//	Initialize();
//}

void Cauldron::Boil()
{
	// 대충 끓는 그래픽과 UI 시작
	if (boilAudio) boilAudio->Play();
	if (VFX_BrewSmoke) VFX_BrewSmoke->isSpawnParticlesByTime = true;

	ui_divide_input->SetTexture(0, ui_paths[5]);
}

void Cauldron::BoilingDone()
{
	// tutorial
	TutorialManagerComponent* tm = TutorialManager::GetInstance();
	if (tm->OnTutorial() == true) tm->ListenEvent(TutorialEventType::BOILING_DONE);


	result_potion_type = RecipeManager::GetInstance()->GetPotionType(current_ingredients);

	// 포션 UI 지우기
	for (int i = 0; i < 2; ++i)
	{
		ing_ui->SmoothDisappear(i);
	}
	ui_background_ing->SmoothDisappear(0);
	ui_mask_ing->SmoothDisappear(0);
	ui_divide_input->SmoothDisappear(0);

	// 대충 완성된 포션 UI
	ui_background_potion->SmoothAppear(0);
	ui_mask_potion->SmoothAppear(0);
	potion_ui->SetTexture(0, ResourceFinder::GetPotionUIPath(result_potion_type));
	potion_ui->SmoothAppear(0);

	overcook_warning_elapsed_time = overcook_warning_interval;
	overcook_warning_show = false;


	if (boilAudio) boilAudio->Stop();
	if (warningAudio) warningAudio->Stop();
	if (VFX_BrewSmoke) VFX_BrewSmoke->isSpawnParticlesByTime = false;
	if (VFX_BrewFail) VFX_BrewFail->isSpawnParticlesByTime = false;
	if (VFX_BrewFinish) VFX_BrewFinish->isSpawnParticlesByTime = true;
}

void Cauldron::Overcooked()
{
	result_potion_type = PotionType::FailurePotion;
	potion_ui->SetTexture(0, ResourceFinder::GetPotionUIPath(result_potion_type));

	overcook_warning_sign->InstantDisappear(0);
	if(failAudio) failAudio->Play();
	if (warningAudio) warningAudio->Stop();
	if (VFX_BrewSmoke) VFX_BrewSmoke->isSpawnParticlesByTime = false;
	if (VFX_BrewFail) VFX_BrewFail->isSpawnParticlesByTime = true;
}

void Cauldron::ThrowFeverPotionToTheCounter()
{
	if (initialize_flying_potion_anchor == false)
	{
		initialize_flying_potion_anchor = true;
		GameObject* counter = GameObject::Find(L"CounterObject");

		if (counter == nullptr)
		{
			use_flying_potion_effect = false;
			return;
		}

		fever_potion_pos_anchor_goal = Camera::GetMainCamera()->WorldToScreenPoint(counter->transform.position) - Camera::GetMainCamera()->WorldToScreenPoint(transform.position);
	}


	for (int i = 0; i < fever_potion_num; ++i)
	{
		if (fever_potion_use[i] == false)
		{
			fever_potion[i]->SmoothAppear(0);
 			fever_potion_use[i] = true;
			fever_potion_t[i] = 0.0f;
			break;
		}
	}

}

Vector4 Cauldron::HSVtoRGB(Vector4 hsv)
{
	float H = hsv.x;
	float S = hsv.y;
	float V = hsv.z;

	if (H > 360 || H < 0 || S>100 || S < 0 || V>100 || V < 0)
	{
		return { 1, 1, 1, 1 };
	}
	float s = S / 100.0f;
	float v = V / 100.0f;
	float C = s * v;
	float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
	float m = v - C;
	float r, g, b;
	if (H >= 0 && H < 60)
	{
		r = C, g = X, b = 0;
	}
	else if (H >= 60 && H < 120)
	{
		r = X, g = C, b = 0;
	}
	else if (H >= 120 && H < 180)
	{
		r = 0, g = C, b = X;
	}
	else if (H >= 180 && H < 240)
	{
		r = 0, g = X, b = C;
	}
	else if (H >= 240 && H < 300)
	{
		r = X, g = 0, b = C;
	}
	else
	{
		r = C, g = 0, b = X;
	}
	float R = (r + m);
	float G = (g + m);
	float B = (b + m);

	return Vector4{ R, G, B , 1.0f };
}