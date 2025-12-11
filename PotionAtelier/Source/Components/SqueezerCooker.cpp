#include "SqueezerCooker.h"
#include <GameObject/Mesh/ParticleObject.h>
#include <Component/Camera/Camera.h>
#include <Component/Render/UIRenderComponenet.h>
#include <Utility/Random.h>

#include <Object/TutorialManager.h>
#include <Object/AudioPlayerObject.h>
#include <Components/PlayerController.h>
#include <Components/Holding.h>
#include <Components/UIPoping.h>
#include <ResourceFinder.h>

SqueezerCooker::SqueezerCooker()
{
	type = InteractableType::Cooker_Squeeze;
}

void SqueezerCooker::Awake()
{
	step = 0;

	const auto& ui_paths = ResourceFinder::GetSqueezeQTEUIPaths();
	for (int i = 0; auto& r : jjuckkk.renderer)
	{
		r = &AddComponent<UIRenderComponenet>();
		r->SetTexture(ui_paths[i++]);		// 0 : BG, 1 : Border, 2 : Section, 3 : Fill
		r->Enable = false;
	}

	icon = &AddComponent<UIPoping>();
	icon->Initialize(1, 0, { 50, 50 }, { 0, -icon_y_delta });
	icon->SetTexture(0, L"./Resource/Icon/IconUI_Squeeze.png");

	LoadVFX();
}

void SqueezerCooker::Start()
{
	GameObject* holding = FindHoldingObject(&transform);
	ingredient_graphic = holding->IsComponent<Holding>();

	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Press"))
	{
		squeeze_sound = findItem->IsComponent<AudioBankClip>();
	}

	std::wstring wstr = std::wstring(cut.begin(), cut.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_Cut = findItem->IsComponent<ParticleSpawnComponent>();
	}
}

void SqueezerCooker::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
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
	if (ImGui::Button("Save"))
	{
		SaveVFX();
	}


	ImGui::PopID();
}

void SqueezerCooker::Update()
{
	if(step == 0)
		return;
	
	UpdatePosition();
	ProcessAnimation();
}

void SqueezerCooker::OnFocusIn(PlayerController* controller)
{
	bool result = IsInteractable(controller);
	if (result == false && !is_cook)
		return;

	on_focus = true;
	if (step == 0 && icon_show == false)
	{
		icon_show = true;
		icon->SmoothAppear(0);
	}
}

void SqueezerCooker::OnFocusOut(PlayerController* controller)
{
	Failed();
	on_focus = false;
	if (step == 0 && icon_show == true)
	{
		icon_show = false;
		icon->SmoothDisappear(0);
	}
}

bool SqueezerCooker::OnInteract(PlayerController* controller)
{
	cached_controller = controller;
	if (step == 0)
	{
		if (IsInteractable(controller) == false)
			return false;

		ingredient_type = (IngredientType)controller->hold_subtype;

		TutorialManagerComponent* tm = TutorialManager::GetInstance();
		if (tm->OnTutorial() == true)
			tm->ListenEvent(TutorialEventType::SQUEEZING_START);

		controller->PutDown();
		GameStart(ingredient_type);

		return true;
	}

	if (step == 1)
	{
		return true;
	}
	
	if (step == 2)
	{
		return true;
	}

	return false;
}

void SqueezerCooker::OnCook(PlayerController* controller)
{
	if (squeeze_sound)
		squeeze_sound->Play();

	if (VFX_Cut)
		VFX_Cut->CreateParticle();

	float fill_x_right = (current_gauge * bar_width);
	float range_x_left = section_x;
	float range_x_right = range_x_left + section_range;

	//printf("fill_x_right: %.2f, range_x_left: %.2f, range_x_right: %.2f\n", fill_x_right, range_x_left, range_x_right);

	squeeze_success = (fill_x_right >= range_x_left && fill_x_right <= range_x_right);

	if (!squeeze_success)
	{
		Failed();
		return;
	}
	for (auto& r : jjuckkk.renderer)
	{
		r->Enable = false;
	}
	if (controller->something_on_hand == false)
	{
		IngredientType compressed = ResourceFinder::GetCompressed(ingredient_type);
		if (compressed & IngredientType::Invalid)
		{
			printf("도대체 뭘 만든거냐 통조림\n");
		}
		else
		{
			TutorialManagerComponent* tm = TutorialManager::GetInstance();
			if (tm->OnTutorial() == true)
				tm->ListenEvent(TutorialEventType::SQUEEZING_DONE);

			controller->Pick(HoldableType::Ingredient, compressed);
		}
	}

	// 부드럽게 사라지기 이펙트?
	if (ingredient_graphic != nullptr)
	{
		ingredient_graphic->SetEmpty();
	}

	Initialize();

}

bool SqueezerCooker::IsInteractable(PlayerController* controller)
{
 	if (controller->something_on_hand == false)
		return false;
	if (controller->hold_type != HoldableType::Ingredient)
		return false;

	IngredientType compressed = ResourceFinder::GetCompressed((IngredientType)controller->hold_subtype);
	if (compressed & IngredientType::Invalid)
		return false;

	TutorialManagerComponent* tm = TutorialManager::GetInstance();
	if (tm->OnTutorial() == true && (tm->lock_interact == true || tm->lock_interact_squeezer == true))
		return false;

	return true;
}

void SqueezerCooker::GameStart(IngredientType type)
{
	if (ingredient_graphic == nullptr)
	{
		GameObject* holdingObj = FindHoldingObject(&transform);
		if (holdingObj != nullptr)
			ingredient_graphic = holdingObj->IsComponent<Holding>();
	}

	if (ingredient_graphic != nullptr)
		ingredient_graphic->SetType(HoldableType::Ingredient, (UINT)ingredient_type);
	
	for (auto& r : jjuckkk.renderer)
	{
		r->Enable = true;
	}

	step = 1;
	is_cook = true;
	UpdatePosition();
	SetSuccessZone();
	ProcessAnimation();

	if (icon_show == true)
	{
		icon->SmoothDisappear(0);
		icon_show = false;
	}
}

void SqueezerCooker::SetSuccessZone()
{
	section_range = Random::Range(section_range_min, section_range_max);
	section_x = Random::Range(section_range * 0.5f +10.f, 100.0f - section_range * 0.5f);

}

void SqueezerCooker::ProcessAnimation()
{

	auto& input = inputManager.input;
	if (!on_focus)
	{
		current_gauge = 0.0f;

	}
	else
	{
		if (input.IsKey(KeyboardKeys::Space))
		{
			current_gauge += fill_speed * TimeSystem::Time.DeltaTime;
		}
		if (input.IsKeyUp(KeyboardKeys::Space))
		{

			OnCook(cached_controller);
		}
		if (current_gauge > 1.0f)
			Failed();
	}

	float fill_width = current_gauge * 100.f;
	float fill_x = std::floor(anchor.x - bar_width * 0.5f + (fill_width * 0.5f) + 5.0f * (1.0f - current_gauge));	// 반올림 처리 안하면 덜덜 거림

	jjuckkk.renderer[3]->SetTransform(fill_x, anchor.y, fill_width, bar_height);


	float fill_width2 = section_range;
	float fill_x2 = std::floor(anchor.x - bar_width * 0.5f + (fill_width2 * 0.5f) + section_x);	// 반올림 처리 안하면 덜덜 거림
	jjuckkk.renderer[2]->SetTransform(fill_x2, anchor.y, fill_width2 , bar_height);	// Section의 위치 랜덤 조정
}

void SqueezerCooker::UpdatePosition()
{

	anchor = Camera::GetMainCamera()->WorldToScreenPoint(transform.position);
	anchor.x -= 20;
	anchor.y -= jjuckk_offset_height;
	for (auto& r : jjuckkk.renderer)
	{
		r->SetTransform(anchor.x, anchor.y, bar_width, bar_height);
	}
}

void SqueezerCooker::Initialize()
{
	step = 0;
	squeeze_success = false;
	current_gauge = 0;

	for (auto& r : jjuckkk.renderer)
	{
		r->Enable = false;
	}

	if (ingredient_graphic != nullptr)
		ingredient_graphic->SetEmpty();
}

void SqueezerCooker::Failed()
{
	current_gauge = 0;
	SetSuccessZone();
}

void SqueezerCooker::SaveVFX()
{
	using namespace Binary;
	if (!std::filesystem::exists(VFX_CutDataPath))
	{
		std::filesystem::create_directories(std::filesystem::path(VFX_CutDataPath).parent_path());
	}
	std::ofstream ofs(VFX_CutDataPath, std::ios::binary | std::ios::trunc);
	{
		constexpr size_t Version = 0;
		Write::data(ofs, Version);

		Write::string(ofs, cut);
	}
	ofs.close();
}

void SqueezerCooker::LoadVFX()
{
	using namespace Binary;
	std::ifstream ifs(VFX_CutDataPath, std::ios::binary);
	if (ifs.is_open())
	{
		size_t Version = 0;
		Version = Read::data<size_t>(ifs);

		if (Version == 0)
		{
			cut = Read::string(ifs);
		}
	}
	ifs.close();
}
