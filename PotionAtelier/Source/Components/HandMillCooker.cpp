#include "HandMillCooker.h"
#include <GameObject/Mesh/ParticleObject.h>
#include <Component/Camera/Camera.h>
#include <Component/Render/UIRenderComponenet.h>
#include <Utility/Random.h>

#include <Object/TutorialManager.h>
#include <Object/AudioPlayerObject.h>
#include <Components/PlayerController.h>
#include <Components/Holding.h>
#include <ResourceFinder.h>
#include "Object/GameManager.h"

using namespace TimeSystem;

HandMillCooker::HandMillCooker()
{
	type = InteractableType::Cooker_Grind;
}

void HandMillCooker::Start()
{
	GameObject* holding = FindHoldingObject(&transform);
	ingredient_graphic = holding->IsComponent<Holding>();

	FindGrinderMesh(&transform);

	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Grind"))
	{
		squeeze_sound = findItem->IsComponent<AudioBankClip>();
	}
	std::wstring wstr = std::wstring(cut.begin(), cut.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_Cut = findItem->IsComponent<ParticleSpawnComponent>();
	}

	ui_background = &AddComponent<UIPoping>();
	ui_background->Initialize(1, 0, ui_img_size, ui_anchor);
	ui_background->bubbles[0]->renderer->drawSpeed = 0;
	ui_background->SetTexture(0, L"./Resource/Textures/QTE/Grinder/UI_QTE_Grind_BG.png");

	for (int i = 0;i < 2; ++i)
	{
		ui_target[i] = &AddComponent<UIPoping>();
		ui_target[i]->Initialize(1, 0, ui_img_size, ui_anchor);
		ui_target[i]->bubbles[0]->renderer->drawSpeed = 1;
		ui_target[i]->SetTexture(0, L"./Resource/Textures/QTE/Grinder/UI_QTE_Grind_Section.png");
	}

	ui_arrow = &AddComponent<UIPoping>();
	ui_arrow->Initialize(1, 0, ui_img_size, ui_anchor);
	ui_arrow->bubbles[0]->renderer->drawSpeed = 2;
	ui_arrow->SetTexture(0, L"./Resource/Textures/QTE/Grinder/UI_QTE_Grind_Arrow.png");

	ui_number_circle = &AddComponent<UIPoping>();
	ui_number_circle->Initialize(1, 0, ui_img_size, ui_anchor);
	ui_number_circle->bubbles[0]->renderer->drawSpeed = 2;
	ui_number_circle->SetTexture(0, L"./Resource/Textures/QTE/Grinder/UI_QTE_Grind_Number.png");

	ui_number = &AddComponent<TextPoping>();
	ui_number->Initialize(ui_number_scale, ui_anchor + ui_number_additional_anchor);
	ui_number->renderer->drawSpeed = 3;
	ui_number->renderer->SetType(FontType::HangameSemiBold);

	icon = &AddComponent<UIPoping>();
	icon->Initialize(1, 0, { 40, 40 }, { 0, -icon_y_delta });
	icon->SetTexture(0, L"./Resource/Icon/IconUI_Grind.png");

	gm = &GameManager::GetGM();

	ui_target_success_tolerance = ui_target_width / ui_center_width;
	ui_target_success_tolerance_half = ui_target_success_tolerance * 0.5f;
}

void HandMillCooker::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("DummyComponent"))
	{
		ImGui::DragFloat2("UI Anchor", &ui_anchor.x);
		ImGui::DragFloat2("Text UI Additional Anchor", &ui_number_additional_anchor.x);
		ImGui::DragFloat("Arrow Speed", &arrow_speed);

		ImGui::TreePop();
	}
	ImGui::InputText("VFX_Cut", (char*)cut.c_str(), cut.size(), ImGuiInputTextFlags_CallbackResize,
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
		}, &cut);
	ImGui::PopID();
}

void HandMillCooker::Serialized(std::ofstream& ofs)
{
	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 2;
	Binary::Write::data(ofs, header); //헤더
	Binary::Write::data(ofs, version); //버전

	if constexpr (version > 0)
	{
		Binary::Write::Vector2(ofs, ui_anchor);
		Binary::Write::Vector2(ofs, ui_number_additional_anchor);
		Binary::Write::data(ofs, arrow_speed);
	}
	if constexpr (version > 1)
	{
		Binary::Write::string(ofs, cut);
	}
}

void HandMillCooker::Deserialized(std::ifstream& ifs)
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

	if (version > 0)
	{
		ui_anchor = Binary::Read::Vector2(ifs);
		ui_number_additional_anchor = Binary::Read::Vector2(ifs);
		arrow_speed = Binary::Read::data<float>(ifs);
	}

	if (version > 1)
	{
		cut = Binary::Read::string(ifs);
	}
}

void HandMillCooker::Update()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay()) return;
#endif
	if (gm->IsTimeEnd())
	{
		icon->InstantDisappear(0);
		ui_background->InstantDisappear(0);
		ui_target[0]->InstantDisappear(0);
		ui_target[1]->InstantDisappear(0);
		ui_arrow->InstantDisappear(0);
		ui_number_circle->InstantDisappear(0);
		ui_number->InstantDisappear();

		return;
	}

	if (step == 0) return;

	if (step == 1)
	{
		float delta = Time.DeltaTime * arrow_speed;
		if (go_right == true)
		{
			t += delta;
			if (t >= 1.0f)
			{
				t = 1 - (t - 1);
				go_right = false;
			}
		}
		else if (go_right == false)
		{
			t -= delta;
			if (t <= 0.0f)
			{
				t = -t;
				go_right = true;
			}
		}
		SetArrowPosition();
	}
}

void HandMillCooker::OnFocusIn(PlayerController* controller)
{
	bool result = IsInteractable(controller);
	if (result == false)
		return;

	on_focus = true;
	if (step == 0 && icon_show == false)
	{
		icon_show = true;
		icon->SmoothAppear(0);
	}
}

void HandMillCooker::OnFocusOut(PlayerController* controller)
{
	on_focus = false;
	if (step == 0 && icon_show == true)
	{
		icon_show = false;
		icon->SmoothDisappear(0);
	}
}

bool HandMillCooker::OnInteract(PlayerController* controller)
{
	if (step == 0) // 재료가 올라가있지 않음
	{
		if (IsInteractable(controller) == false)
			return false;

		ingredient_type = (IngredientType)controller->hold_subtype;

		TutorialManagerComponent* tm = TutorialManager::GetInstance();
		if (tm->OnTutorial() == true)
			tm->ListenEvent(TutorialEventType::GRINDING_START);

		controller->PutDown();
		GameStart(ingredient_type);
		if (grinder_mesh != nullptr) grinder_mesh->Active = false;

		return true;
	}
	else if (step == 1) // 재료가 올라간 뒤 조리할 수 있는 상태
	{
		if (controller->something_on_hand == true) return false;

		OnCook(controller);

		controller->PlayCookingAnimation(type);

		if (sub_step < 4)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

void HandMillCooker::GameStart(IngredientType type)
{
	if (ingredient_graphic == nullptr)
	{
		GameObject* holdingObj = FindHoldingObject(&transform);
		if (holdingObj != nullptr)
			ingredient_graphic = holdingObj->IsComponent<Holding>();
	}

	if (ingredient_graphic != nullptr)
		ingredient_graphic->SetType(HoldableType::Ingredient, (UINT)ingredient_type);

	ui_background->SmoothAppear(0);
	ui_target[0]->SmoothAppear(0);
	ui_target[1]->SmoothAppear(0);
	ui_arrow->SmoothAppear(0);
	ui_number_circle->SmoothAppear(0);
	ui_number->SmoothAppear();
	ui_number->SetText(L"0");

	step = 1;
	sub_step = 0;
	t = 0.0f;
	go_right = true;

	InitializeTargetClearFlag();
	SetTargetPosition();
	SetArrowPosition();

	if (icon_show == true)
	{
		icon->SmoothDisappear(0);
		icon_show = false;
	}
}

bool HandMillCooker::IsInteractable(PlayerController* controller)
{
	if (controller->something_on_hand == false)
		return false;
	if (controller->hold_type != HoldableType::Ingredient)
		return false;

	IngredientType ground = ResourceFinder::GetGround((IngredientType)controller->hold_subtype);
	if (ground & IngredientType::Invalid)
		return false;

	TutorialManagerComponent* tm = TutorialManager::GetInstance();
	if (tm->OnTutorial() == true && (tm->lock_interact == true || tm->lock_interact_grinder == true))
		return false;

	return true;
}

void HandMillCooker::SetTargetPosition()
{
	bool target_cleared[2]{ true, true };
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			if (target_clear[i][j] == true)
			{
				continue;
			}
			else
			{
				float half_width = ui_center_width * 0.5f;
				float anchor_x = (target_pos[i][j] - 0.5f) * 2 * half_width;
				ui_target[i]->anchor.x = anchor_x;
				target_cleared[i] = false;
				break;
			}
		}
	}

	if (target_cleared[0] == true) ui_target[0]->InstantDisappear(0);
	if (target_cleared[1] == true) ui_target[1]->InstantDisappear(0);
}

void HandMillCooker::SetArrowPosition()
{
	float half_width = ui_center_width * 0.5f;
	float anchor_x = (t - 0.5f) * 2 * half_width;
	ui_arrow->anchor.x = anchor_x + 1;
	ui_number_circle->anchor.x = anchor_x + 1;
	ui_number->anchor.x = anchor_x + 1 + ui_number_additional_anchor.x;
}

void HandMillCooker::InitializeTargetClearFlag()
{
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			target_clear[i][j] = false;
		}
	}
}

void HandMillCooker::OnCook(PlayerController* controller)
{
	if (squeeze_sound) squeeze_sound->Play();
	
	if (VFX_Cut) VFX_Cut->CreateParticle();

	bool clear{ false };
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			if (target_clear[i][j] == true) continue;

			float left_threshold = target_pos[i][j] - ui_target_success_tolerance_half;
			float right_threshold = target_pos[i][j] + ui_target_success_tolerance_half;

			if (t >= left_threshold && t <= right_threshold)
			{
				target_clear[i][j] = true;
				++sub_step;

				ui_number->SetText(std::format(L"{}", sub_step));

				SetTargetPosition();
				clear = true;
			}
			break;
		}
		if (clear) break;
	}

	if (sub_step == 3)
	{
		++step;
		JobDone(controller);
	}
}

void HandMillCooker::JobDone(PlayerController* controller)
{
	if (controller->something_on_hand == true) return;

	IngredientType ground_ing = ResourceFinder::GetGround(ingredient_type);
	if (ground_ing & IngredientType::Invalid)
	{
		printf("이 커맨드는 진구만 가능한데...\n");
	}
	else
	{
		TutorialManagerComponent* tm = TutorialManager::GetInstance();
		if (tm->OnTutorial() == true)
			tm->ListenEvent(TutorialEventType::GRINDING_DONE);

		controller->Pick(HoldableType::Ingredient, ground_ing);
	}

	if (ingredient_graphic != nullptr) ingredient_graphic->SetEmpty();
	if (grinder_mesh != nullptr) grinder_mesh->Active = true;
	Initialize();

	return;
}

void HandMillCooker::Initialize()
{
	step = 0;
	sub_step = 0;
	t = 0.0f;
	go_right = true;

	ingredient_graphic->SetEmpty();

	ui_background->SmoothDisappear(0);
	ui_target[0]->SmoothDisappear(0);
	ui_target[1]->SmoothDisappear(0);
	ui_arrow->SmoothDisappear(0);
	ui_number_circle->SmoothDisappear(0);
	ui_number->SmoothDisappear();

	InitializeTargetClearFlag();
}


void HandMillCooker::FindGrinderMesh(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"Grinder"))
		{
			grinder_mesh = &c->gameObject;
		}
		if (grinder_mesh != nullptr) return;
		else FindGrinderMesh(c);
	}
}


