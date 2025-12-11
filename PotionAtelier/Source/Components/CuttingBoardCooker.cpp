#include "CuttingBoardCooker.h"
#include "PlayerController.h"
#include "Holding.h"
#include "../ResourceFinder.h"
#include "UIPoping.h"
#include "../Object/TutorialManager.h"
#include "Object\AudioPlayerObject.h"
#include "Object/GameManager.h"

using namespace TimeSystem;

CuttingBoardCooker::CuttingBoardCooker()
{
	type = InteractableType::Cooker_Cut;
}

void CuttingBoardCooker::Awake()
{
	nyams.resize(divide_num);
	step = 0;

	for (int i = 0; i < divide_num; ++i)
	{
		nyams[i].renderer = &AddComponent<UIRenderComponenet>();
		nyams[i].renderer->Enable = false;
	}

	icon = &AddComponent<UIPoping>();
	icon->Initialize(1, 0, { 50, 50 }, { 0, -icon_y_delta });
	icon->SetTexture(0, L"./Resource/Icon/IconUI_Cut.png");
}

void CuttingBoardCooker::Start()
{
	GameObject* holding = FindHoldingObject(&transform);
	ingredient_graphic = holding->IsComponent<Holding>();


	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Cut"))
	{
		cut_sound = findItem->IsComponent<AudioBankClip>();
	}
	std::wstring wstr = std::wstring(cut.begin(), cut.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_Cut = findItem->IsComponent<ParticleSpawnComponent>();
	}



	gm = &GameManager::GetGM();
}

void CuttingBoardCooker::OnFocusIn(PlayerController* controller)
{
	// 사용 가능할 때만 UI 아이콘 띄우기
	bool result = IsInteractable(controller);
	if (result == false) return;
	
	on_focus = true;
	if (step == 0 && icon_show == false)
	{
		icon_show = true;
		icon->SmoothAppear(0);
	}
	//transform.SetScale({ 1.1f, 1.1f, 1.1f });
}

void CuttingBoardCooker::OnFocusOut(PlayerController* controller)
{
	on_focus = false;
	if (step == 0 && icon_show == true)
	{
		icon_show = false;
		icon->SmoothDisappear(0);
	}
	//transform.SetScale({ 1.0f, 1.0f, 1.0f });
}

bool CuttingBoardCooker::OnInteract(PlayerController* controller)
{
	// step 0 -> 아무것도 올려지지 않음
	if (step == 0)
	{
		bool is_interactable = IsInteractable(controller);
		if (is_interactable == false) return false;

		ingredient_type = (IngredientType)controller->hold_subtype;

		// tutorial
		TutorialManagerComponent* tm = TutorialManager::GetInstance();
		if (tm->OnTutorial() == true) tm->ListenEvent(TutorialEventType::CUTTING_START);

		controller->PutDown();
		GameStart(ingredient_type);

		controller->PlayCookingAnimation(type);

		return true;
	}
	// 무언가가 올려진 뒤 조리할 수 있는 상태
	else if (step == 1)
	{
		if (controller->something_on_hand == true) return false;

		OnCook(controller);

		controller->PlayCookingAnimation(type);

		return false;
	}
	// 조리가 완료된 뒤 가공된 재료를 가져갈 수 있는 상태
	else if (step == 2)
	{
		if (controller->something_on_hand == true) return false;

		IngredientType type = ResourceFinder::GetSliced(ingredient_type);
		if (type == IngredientType::Invalid)
		{
			printf("머임??? 대체 머임???\n");
		}
		else
		{
			// tutorial
			TutorialManagerComponent* tm = TutorialManager::GetInstance();
			if (tm->OnTutorial() == true) tm->ListenEvent(TutorialEventType::CUTTING_DONE);

			controller->Pick(HoldableType::Ingredient, type);
			controller->InformQTEDone();
		}


		{
			// 부드럽게 사라지기
			step++;
			disappearing = true;
			scale_anim_t = 0.0f;
			if (ingredient_graphic != nullptr) ingredient_graphic->SetEmpty();

			/*if (on_focus && icon_show == false)
			{
				icon_show = true;
				icon->SmoothAppear(0);
			}*/
		}
		return true;
	}
}









void CuttingBoardCooker::Update()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay()) return;
#endif
	if (gm->IsTimeEnd())
	{
		for (int i = 0; i < divide_num; ++i)
		{
			nyams[i].renderer->Enable = false;
			icon->InstantDisappear(0);
		}
	}

	if (step == 0) return;

	ProcessAnimation();
	UpdatePartsBasePosition();
	SetPartsPosition();
}

void CuttingBoardCooker::InputTexture(IngredientType type)
{
	std::vector<std::wstring> paths = ResourceFinder::GetIngredientSlicedPartsUIPaths(type);

	if (divide_num != paths.size())
	{
		__debugbreak;
	}

	for (int i = 0; i < divide_num; ++i)
	{
		nyams[i].renderer->SetTexture(paths[i]);
	}
}

void CuttingBoardCooker::Initialize()
{
	step = 0;
	sub_step = 0;
	scale_anim_t = 0.0f;
	disappearing = false;

	for (int i = 0; i < divide_num; ++i)
	{
		nyams[i].t = 0.0f;
		nyams[i].is_cut = false;
		nyams[i].renderer->Enable = false;
	}

	if (ingredient_graphic != nullptr) ingredient_graphic->SetEmpty();
}

void CuttingBoardCooker::UpdatePartsBasePosition()
{
	NyamNyamAnchor = Camera::GetMainCamera()->WorldToScreenPoint(transform.position);
	NyamNyamAnchor.x += image_anchor.x;
	NyamNyamAnchor.y -= image_anchor.y;


	float position_x;
	if (divide_num % 2 == 0)
	{
		position_x = NyamNyamAnchor.x - ((divide_num / 2 + 0.5f) * part_gap_px);
	}
	else
	{
		position_x = NyamNyamAnchor.x - (divide_num / 2 * part_gap_px);
	}

	for (int i = 0; i < divide_num; ++i)
	{
		nyams[i].original_position = { position_x, NyamNyamAnchor.y };
		nyams[i].goal_position.x = position_x - flew_distance.x;
		nyams[i].goal_position.y = nyams[i].original_position.y;
		if (i % 2 == 0)
		{
			nyams[i].goal_position.y -= flew_distance.y;
			nyams[i].goal_rotation = -flew_rotation;
		}
		else
		{
			nyams[i].goal_position.y += flew_distance.y;
			nyams[i].goal_rotation = flew_rotation;
		}

		// 위치 조정
		if (ingredient_type == IngredientType::DragonTail)
		{
			nyams[i].original_position += dragon_tail_add_pos[i];
			nyams[i].goal_position += dragon_tail_add_pos[i];
		}
		else if (ingredient_type == IngredientType::MagicWood)
		{
			nyams[i].original_position += magic_wood_add_pos[i];
			nyams[i].goal_position += magic_wood_add_pos[i];
		}

		position_x += part_gap_px;
	}
}

void CuttingBoardCooker::SetPartsPosition()
{
	for (int i = 0; i < divide_num; ++i)
	{
		if (nyams[i].is_cut == false)
		{
			nyams[i].renderer->SetTransform(nyams[i].original_position.x, nyams[i].original_position.y, part_width_px * part_size_mult, part_height_px * part_size_mult);
		}
		else
		{
			Vector2 cur_pos = Vector2::Lerp(nyams[i].original_position, nyams[i].goal_position, nyams[i].t);

			// 위치 조정
			if (ingredient_type == IngredientType::DragonTail)
			{
				cur_pos += dragon_tail_add_pos[i];
			}
			else if (ingredient_type == IngredientType::MagicWood)
			{
				cur_pos += magic_wood_add_pos[i];
			}

			float cur_rotation = std::lerp(nyams[i].original_rotation, nyams[i].goal_rotation, nyams[i].t);
			nyams[i].renderer->SetTransform(cur_pos.x, cur_pos.y, part_width_px * part_size_mult, part_height_px * part_size_mult, cur_rotation);
		}
	}
}

void CuttingBoardCooker::GameStart(IngredientType type)
{
	if (ingredient_graphic == nullptr)
	{
		auto* holdingObj = FindHoldingObject(&transform);
		if (holdingObj != nullptr)
		{
			ingredient_graphic = holdingObj->IsComponent<Holding>();
			//if (ingredient_graphic == nullptr) return;
		}
	}

	if (ingredient_graphic != nullptr)
	{
		ingredient_graphic->SetType(HoldableType::Ingredient, (UINT)ingredient_type);
	}

	for (int i = 0; i < divide_num; ++i)
	{
		nyams[i].renderer->Enable = true;
	}

	step = 1;
	InputTexture(type);
	ProcessAnimation();

	if (icon_show == true)
	{
		icon->SmoothDisappear(0);
		icon_show = false;
	}
}

void CuttingBoardCooker::OnCook(PlayerController* controller)
{
	if (cut_sound) cut_sound->Play();
	if (VFX_Cut) VFX_Cut->CreateParticle();
	if (sub_step < divide_num)
	{
		nyams[sub_step].is_cut = true;
		++sub_step;
	}

	if (sub_step == divide_num - 1)
	{
		// 대충 조리가 완료됨
		// 대충 가공 완료된 재료 메쉬 띄울 거면 여기서
		++step;
	}
}

bool CuttingBoardCooker::IsInteractable(PlayerController* controller)
{
	if (controller->something_on_hand == false) return false;
	if (controller->hold_type != HoldableType::Ingredient) return false;

	IngredientType sliced = ResourceFinder::GetSliced((IngredientType)controller->hold_subtype);
	if (sliced == IngredientType::Invalid) return false;

	// tutorial
	TutorialManagerComponent* tm = TutorialManager::GetInstance();
	if (tm->OnTutorial() == true && (tm->lock_interact == true || tm->lock_interact_cutting_board == true)) return false;

	return true;
}

void CuttingBoardCooker::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("CuttingBoardCooker"))
	{
		ImGui::SliderFloat("Image Size", &part_size_mult, 0.0f, 3.0f);
		ImGui::SliderFloat2("Image Anchor", &image_anchor.x, -100.0f, 100.0f);
		ImGui::SliderFloat("Image Gap", &goal_gap, 20.0f, 60.0f);
		ImGui::SliderFloat2("Fly Distance", &flew_distance.x, 0.0f, 60.0f);
		ImGui::SliderFloat("Fly Speed", &flew_speed, 0.0f, 60.0f);
		ImGui::SliderFloat("Fly Rotation", &flew_rotation, 0.0f, 60.0f);

		ImGui::Dummy(ImVec2{ 0, 10 });
		ImGui::SliderFloat2("Tail_AddPos1", &dragon_tail_add_pos[0].x, -10.0f, 10.0f);
		ImGui::SliderFloat2("Tail_AddPos2", &dragon_tail_add_pos[1].x, -10.0f, 10.0f);
		ImGui::SliderFloat2("Tail_AddPos3", &dragon_tail_add_pos[2].x, -10.0f, 10.0f);
		ImGui::SliderFloat2("Tail_AddPos4", &dragon_tail_add_pos[3].x, -10.0f, 10.0f);
		ImGui::SliderFloat2("Tail_AddPos5", &dragon_tail_add_pos[4].x, -10.0f, 10.0f);

		ImGui::Dummy(ImVec2{ 0, 10 });
		ImGui::SliderFloat2("Wood_AddPos1", &magic_wood_add_pos[0].x, -10.0f, 10.0f);
		ImGui::SliderFloat2("Wood_AddPos2", &magic_wood_add_pos[1].x, -10.0f, 10.0f);
		ImGui::SliderFloat2("Wood_AddPos3", &magic_wood_add_pos[2].x, -10.0f, 10.0f);
		ImGui::SliderFloat2("Wood_AddPos4", &magic_wood_add_pos[3].x, -10.0f, 10.0f);
		ImGui::SliderFloat2("Wood_AddPos5", &magic_wood_add_pos[4].x, -10.0f, 10.0f);

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

void CuttingBoardCooker::Serialized(std::ofstream& ofs)
{
	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 0;
	Binary::Write::data(ofs, header); //헤더
	Binary::Write::data(ofs, version); //버전

	Binary::Write::data(ofs, part_size_mult);
	Binary::Write::Vector2(ofs, image_anchor);
	Binary::Write::data(ofs, goal_gap);
	Binary::Write::Vector2(ofs, flew_distance);
	Binary::Write::data(ofs, flew_speed);
	Binary::Write::data(ofs, flew_rotation);
	Binary::Write::Vector2(ofs, dragon_tail_add_pos[0]);
	Binary::Write::Vector2(ofs, dragon_tail_add_pos[1]);
	Binary::Write::Vector2(ofs, dragon_tail_add_pos[2]);
	Binary::Write::Vector2(ofs, dragon_tail_add_pos[3]);
	Binary::Write::Vector2(ofs, dragon_tail_add_pos[4]);
	Binary::Write::Vector2(ofs, magic_wood_add_pos[0]);
	Binary::Write::Vector2(ofs, magic_wood_add_pos[1]);
	Binary::Write::Vector2(ofs, magic_wood_add_pos[2]);
	Binary::Write::Vector2(ofs, magic_wood_add_pos[3]);
	Binary::Write::Vector2(ofs, magic_wood_add_pos[4]);
}

void CuttingBoardCooker::Deserialized(std::ifstream& ifs)
{
	// 터지면 이거 주석하고 열고 저장하고 주석 풀고
	size_t header = Binary::Read::data<size_t>(ifs);
	uint32_t version = Binary::Read::data<uint32_t>(ifs);

	part_size_mult = Binary::Read::data<float>(ifs);
	image_anchor = Binary::Read::Vector2(ifs);
	goal_gap = Binary::Read::data<float>(ifs);
	flew_distance = Binary::Read::Vector2(ifs);
	flew_speed = Binary::Read::data<float>(ifs);
	flew_rotation = Binary::Read::data<float>(ifs);
	dragon_tail_add_pos[0] = Binary::Read::Vector2(ifs);
	dragon_tail_add_pos[1] = Binary::Read::Vector2(ifs);
	dragon_tail_add_pos[2] = Binary::Read::Vector2(ifs);
	dragon_tail_add_pos[3] = Binary::Read::Vector2(ifs);
	dragon_tail_add_pos[4] = Binary::Read::Vector2(ifs);
	magic_wood_add_pos[0] = Binary::Read::Vector2(ifs);
	magic_wood_add_pos[1] = Binary::Read::Vector2(ifs);
	magic_wood_add_pos[2] = Binary::Read::Vector2(ifs);
	magic_wood_add_pos[3] = Binary::Read::Vector2(ifs);
	magic_wood_add_pos[4] = Binary::Read::Vector2(ifs);
}

void CuttingBoardCooker::ProcessAnimation()
{
	if (disappearing == false)
	{
		scale_anim_t += Time.DeltaTime * scale_anim_speed;
		if (scale_anim_t >= 1.0f) scale_anim_t = 1.0f;
		part_gap_px = std::lerp(0.0f, goal_gap, scale_anim_t);
		part_width_px = std::lerp(0.0f, goal_width, scale_anim_t);
		part_height_px = std::lerp(0.0f, goal_height, scale_anim_t);
	}
	else if (disappearing == true)
	{
		scale_anim_t += Time.DeltaTime * scale_anim_speed;
		if (scale_anim_t >= 1.0f)
		{
			Initialize();
		}
		part_gap_px = std::lerp(goal_gap, 0.0f, scale_anim_t);
		part_width_px = std::lerp(goal_width, 0.0f, scale_anim_t);
		part_height_px = std::lerp(goal_height, 0.0f, scale_anim_t);
	}

	for (int i = 0; i < divide_num; ++i)
	{
		if (nyams[i].is_cut == false) break;

		nyams[i].t += Time.DeltaTime * flew_speed;
		if (nyams[i].t >= 1.0f) nyams[i].t = 1.0f;
	}
}
