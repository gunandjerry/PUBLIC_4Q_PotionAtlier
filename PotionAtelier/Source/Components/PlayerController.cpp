#include "PlayerController.h"
#include "Interactable.h"
#include "Throwable.h"
#include "Holding.h"
#include "Cooker.h"
#include "Utility/SerializedUtility.h"
#include "BoingBoing.h"
#include "../Object/GameManager.h"
#include "../Object/TutorialManager.h"
#include "TextBubble.h"
#include <Object\AudioPlayerObject.h>
#include "UIPoping.h"
#include "BroomComponent.h"

using namespace TimeSystem;

void PlayerControllHelper::PlayAnimation(PlayerAnimType type)
{
	if (animator == nullptr) return;

	if (lock_change_animation == true) return;
	if (lock_change_animation_until_focus_out_or_QTE_done == true) return;

	controller->knife->Active = false;
	controller->grinder->Active = false;

	switch (type)
	{
	case PlayerAnimType::Attack_With_Broom:
		animator->PlayClip(ResourceFinder::GetPlayerAnimationClipName(type), false);
		break;
	case PlayerAnimType::Chopping:
		controller->knife->Active = true;
		animator->PlayClip(ResourceFinder::GetPlayerAnimationClipName(type), true);
		break;
	case PlayerAnimType::Grinding:
		controller->grinder->Active = true;
		animator->PlayClip(ResourceFinder::GetPlayerAnimationClipName(type), true);
		break;
	case PlayerAnimType::Idle:
	case PlayerAnimType::Idle_Hold_Broom:
	case PlayerAnimType::Move:
	case PlayerAnimType::Move_Hold_Normal:
	case PlayerAnimType::Move_Hold_Broom:
	case PlayerAnimType::Move_Hold_Broom_Scale:
	case PlayerAnimType::Squeezing:
	case PlayerAnimType::BlowingBellows:
		animator->PlayClip(ResourceFinder::GetPlayerAnimationClipName(type), true);
	}
}








void PlayerController::Awake() {
	current_scene = SceneManager::GetInstance().GetActiveScene();
	current_scene->update_physics_scene = false;


	controller = &GetComponent<CharacterController>();
	VFX_Run = &AddComponent<ParticleSpawnComponent>();
	VFX_Run->isSpawnParticlesByTime = false;
}

void PlayerController::Start()
{
	auto ui_paths = ResourceFinder::GetStaminaUI();

	ui_stamina_background = &AddComponent<UIPoping>();
	ui_stamina_background->Initialize(1, 0, ui_stamina_size, ui_stamina_anchor);
	ui_stamina_background->SetTexture(0, ui_paths[0]);
	ui_stamina_background->bubbles[0]->renderer->drawSpeed = -12;

	ui_stamina_fill = &AddComponent<UIPoping>();
	ui_stamina_fill->Initialize(1, 0, ui_stamina_fill_size, ui_stamina_fill_anchor);
	ui_stamina_fill->SetTexture(0, ui_paths[1]);
	ui_stamina_fill->bubbles[0]->renderer->drawSpeed = -11;

	stamina = max_stamina;


	gm = &GameManager::GetGM();

	if (hand_graphic == nullptr) hand_graphic = FindHoldingObject(&transform);
	if (player_animator == nullptr) FindAnimationObject(&transform);
	if (broom == nullptr) FindBroom(&transform);
	if (knife == nullptr) FindKnife(&transform);
	if (grinder == nullptr) FindGrinder(&transform);

	broom_scale_goal = broom->transform.GetLocalScale();
	broom->transform.SetLocalScale(broom_scale_init);

	tutorial_manager = TutorialManager::GetInstance();
	if (tutorial_manager == nullptr)
	{
		__debugbreak();
	}
	if (gm->GetCurrentStageNum() != 0)
		GameObject::Destroy(tutorial_manager->gameObject);

	helper.animator = player_animator;
	helper.controller = this;

	superficial_rotation = Mathf::PI;
	if (player_animator != nullptr)
	{
		if (something_on_hand)
		{
			helper.PlayAnimation(PlayerAnimType::Idle);
		}
		else
		{
			helper.PlayAnimation(PlayerAnimType::Idle);
		}
	}


	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Hold"))
	{
		holdAudio = findItem->IsComponent<AudioBankClip>();
	}
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Drop"))
	{
		dropAudio = findItem->IsComponent<AudioBankClip>();
	}
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_NotValid"))
	{
		SFX_NotValid = findItem->IsComponent<AudioBankClip>();
	}
	std::wstring wstr = std::wstring(run.begin(), run.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_Run = findItem->IsComponent<ParticleSpawnComponent>();
	}
	if (VFX_Run) VFX_Run->isSpawnParticlesByTime = false;
}

void PlayerController::Update()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay()) return;
#endif

	if (gm->IsTimeEnd())
	{
		GetComponent<TextBubble>().HideBubble();
		ui_stamina_background->InstantDisappear(0);
		ui_stamina_fill->InstantDisappear(0);

		if (current_scene->update_physics_scene == true)
		{
			current_scene->update_physics_scene = false;
		}
	}

	if (GameManager::GetGM().IsStageStart() == false) return;

	if (current_scene->update_physics_scene == false)
	{
		current_scene->update_physics_scene = true;
	}

	interact_at_current_frame = false;

	
	if (stamina == max_stamina)
	{
		if (stamina_ui_show == true)
		{
			stamina_ui_show = false;
			HideStaminaUI();
		}
	}
	else
	{
		AdjustStaminaUI();
		if (stamina_ui_show == false)
		{
			stamina_ui_show = true;
			ShowStaminaUI();
		}
	}



	if (on_attack)
	{
		if (broom_showing_t < 1.0f)
		{
			broom_showing_t += Time.DeltaTime * broom_showing_speed;
			broom->transform.SetLocalScale(Vector3::Lerp(broom_scale_init, broom_scale_goal, broom_showing_t));
		}
		else if (broom_showing_t >= 1.0f && broom_showing_t < 2.0f)
		{
			broom_showing_t += Time.DeltaTime / broom_maintain_sec;
		}
		else if (broom_showing_t >= 2.0f && broom_showing_t < 3.0f)
		{
			broom_showing_t += Time.DeltaTime * broom_showing_speed;
			broom->transform.SetLocalScale(Vector3::Lerp(broom_scale_init, broom_scale_goal, 1 - (broom_showing_t - 2.0f)));
		}
		else if (broom_showing_t >= 3.0f)
		{
			broom->gameObject.Active = false;
			on_attack = false;

			helper.lock_move = false;
			helper.lock_interact = false;
			helper.lock_attack = false;
			helper.lock_change_animation = false;

			helper.PlayAnimation(PlayerAnimType::Idle);
		}
	}


	Move();
	RayForward();
	Interact();
}

void PlayerController::Move()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay()) return;
#endif

	auto& input = inputManager.input;
	float dt = Time.DeltaTime;

	if (tutorial_manager != nullptr && tutorial_manager->OnTutorial() == true)
	{
		if (tutorial_manager->lock_move == true)
		{
			if (player_animator == nullptr) FindAnimationObject(&transform);

			if (something_on_hand == true)
			{
				helper.PlayAnimation(PlayerAnimType::Idle);
			}
			else
			{
				helper.PlayAnimation(PlayerAnimType::Idle);
			}

			stamina += stamina_recover_amount_per_second * dt;
			if (stamina > max_stamina) stamina = max_stamina;

			return;
		}
	}

	if (helper.lock_move)
	{
		stamina += stamina_recover_amount_per_second * dt;
		if (stamina > max_stamina) stamina = max_stamina;
		helper.PlayAnimation(PlayerAnimType::Idle);

		return;
	}



	Vector3 movement{ 0, 0, 0 };
	if (input.IsKey(KeyboardKeys::UpArrow)) movement.z += 1;
	if (input.IsKey(KeyboardKeys::DownArrow)) movement.z += -1;
	if (input.IsKey(KeyboardKeys::LeftArrow)) movement.x += -1;
	if (input.IsKey(KeyboardKeys::RightArrow)) movement.x += 1;
	if (movement.LengthSquared() > 0.0f)
	{
		movement.Normalize();
	}
	movement *= max_speed;

	if (movement.LengthSquared() > 0.0f && VFX_Run)
	{
		VFX_Run->position = transform.GetPosition();
		VFX_Run->velocity = -movement;
		VFX_Run->velocity.Normalize();
		VFX_Run->velocity *= 3;


		if (!VFX_Run->isSpawnParticlesByTime)
		{
			for (size_t i = 0; i < 10; i++)
			{
				VFX_Run->CreateParticle(transform.GetPosition());
			}
		}
		VFX_Run->isSpawnParticlesByTime = true;
	}
	else
	{
		VFX_Run->isSpawnParticlesByTime = false;
	}


	bool on_dash{ false };
	
	if (movement.x != 0.0f || movement.y != 0.0f || movement.z != 0.0f)
	{
		if (input.IsKey(dash_key))
		{
			// 한 번 고갈되면 키를 뗐다가 다시 눌러야 함
			if (stamina_exhausted == true)
			{

			}
			else if (stamina_exhausted == false)
			{
				on_dash = true;
			}
		}
		else
		{
			if (stamina_exhausted == true)
			{
				stamina_exhausted = false;
			}
		}


		float theta = std::atan2f(movement.x, movement.z);
		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(0, theta, 0);
		separated_interact_direction = XMVector3Transform(XMVectorSet(0, 0, 1, 0), rotationMatrix);

		float pi2 = 2 * Mathf::PI;

		superficial_rotation = std::fmod(superficial_rotation, pi2);
		if (superficial_rotation < 0) superficial_rotation += pi2;

		float delta_angle = std::fmod(theta - superficial_rotation + pi2, pi2);
		if (std::abs(delta_angle) > Mathf::PI) delta_angle -= pi2 * (delta_angle > 0 ? 1 : -1);

		float rotate_amount = rotate_speed * dt;
		if (smooth_rotation)
		{
			if (fasten_rotation_speed_by_delta_angle)
			{
				float factor = 1 + (1 - std::pow((delta_angle / Mathf::PI), 2));
				rotate_amount *= factor;
			}
			else
			{
				rotate_amount *= 2;
			}
		}
		else
		{
			rotate_amount = 999;
		}

		if (std::fabs(delta_angle) < rotate_amount)
		{
			superficial_rotation = theta;
		}
		else
		{
			superficial_rotation += (delta_angle > 0 ? 1 : -1) * rotate_amount;
		}

		superficial_rotation = std::fmod(superficial_rotation, pi2);
		if (superficial_rotation < 0) superficial_rotation += pi2;

		transform.SetRotation({ 0, superficial_rotation * Mathf::Rad2Deg, 0 });


		// controller->Move({ move_horizontally, 0, move_vertically });

		// deltaAngle에 따라 이동 속도 조절
		float factor = 1.0f;
		if (fasten_rotation_speed_by_delta_angle)
		{
			factor = 1 - std::pow((delta_angle / Mathf::PI), 2);
			rotate_amount *= factor;
		}


		if (on_dash)
		{
			float need_stamina = dash_consume_stamina_per_second * dt;
			if (stamina < need_stamina)
			{
				stamina_exhausted = true;
			}
			else
			{
				stamina -= need_stamina;
				factor *= dash_speed_increase_mult;

				if (tutorial_manager != nullptr && tutorial_manager->OnTutorial() == true) tutorial_manager->ListenEvent(TutorialEventType::DASH);
			}
		}


		controller->Move(movement * factor);
		

		if (player_animator == nullptr) FindAnimationObject(&transform);
		if (player_animator != nullptr)
		{
			if (something_on_hand)
			{
				helper.PlayAnimation(PlayerAnimType::Move_Hold_Normal);
			}
			else
			{
				helper.PlayAnimation(PlayerAnimType::Move);
			}
		}
	}
	else
	{
		if (player_animator == nullptr) FindAnimationObject(&transform);
		if (player_animator != nullptr)
		{
			if (something_on_hand)
			{
				helper.PlayAnimation(PlayerAnimType::Idle);
			}
			else
			{
				helper.PlayAnimation(PlayerAnimType::Idle);
			}
		}
	}

	if (on_dash == false)
	{
		stamina += stamina_recover_amount_per_second * dt;
		if (stamina > max_stamina) stamina = max_stamina;
	}
}

void PlayerController::RayForward()
{
	Vector3 ray_direction;
	if (separate_interact_direction)
	{
		ray_direction = separated_interact_direction;
	}
	else
	{
		ray_direction = transform.Forward;
	}

	RaycastResult result = Physics::Sweep(SweepShape::Sphere, ray_size.x, ray_size.y, transform.GetPosition(), ray_direction, ray_distance, 1, &GetGameObject());
	//RaycastResult result = PhysicsManager::GetInstance().Raycast(transform.GetPosition(), interact_direction, 3.0f, 1, &GetGameObject());
	if (!result.hits.empty()) {
		GameObject* object = result.hits[0].object;

		Interactable* ib = object->_GetInteractableComponent();
		Holding* new_focus_holding = FindHoldingObject(&object->transform);
		if (ib != nullptr) {
			if (current_focus != nullptr && ib != current_focus)
			{
				current_focus->OnFocusOut(this);
				helper.lock_change_animation_until_focus_out_or_QTE_done = false;

				if (focus_holding)
				{
					focus_holding->SetFocus(false);
					focus_holding = new_focus_holding;
				}

				current_focus = ib;
				ib->OnFocusIn(this);
				//current_focus = nullptr;
				//focus_holding = nullptr;
			}
			else if (ib == current_focus)
			{

			}
			else
			{
				current_focus = ib;
				ib->OnFocusIn(this);
			}
		}
		if (focus_holding) focus_holding->SetFocus(true);
	}
	else {
		if (current_focus != nullptr) {
			current_focus->OnFocusOut(this);
			helper.lock_change_animation_until_focus_out_or_QTE_done = false;
		}
		if (focus_holding != nullptr)
		{
			focus_holding->SetFocus(false);
			focus_holding = nullptr;
		}
		current_focus = nullptr;
	}
}

void PlayerController::Interact()
{
	auto& input = inputManager.input;

	if (input.IsKeyDown(interact_key) && current_focus != nullptr)
	{
		if (tutorial_manager != nullptr && tutorial_manager->OnTutorial() == true)
		{
			bool can_interact = tutorial_manager->CanInteract(current_focus);
			if (can_interact == false) return;
		}

		if (helper.lock_interact) return;

		bool result = current_focus->OnInteract(this);

		if (result == true)
		{
			BoingBoing* boing = current_focus->IsComponent<BoingBoing>();
			if (boing != nullptr) boing->Boing(0);
		}


		if (result == false)
		{
			if (current_focus)
			{
				if (SFX_NotValid) SFX_NotValid->Play();
			}		}
		else
		{
			interact_at_current_frame = true;
		}
	}
	else if (input.IsKeyDown(attack_key) && on_attack == false)
	{
		if (helper.lock_attack) return;

		// focus가 있으면 맞고 없으면 모션만
		// 때리는 모션 중엔 못 움직이게 고정?
		if (broom == nullptr) FindBroom(&transform);

		if (current_focus == nullptr)
		{
			// Poo
		}
		else
		{
			current_focus->OnAttacked(this);
		}

		if (broom->gameObject.Active == false) broom->gameObject.Active = true;
		broom_showing_t = 0.0f;
		on_attack = true;

		if (helper.lock_change_animation_until_focus_out_or_QTE_done == true)
		{
			helper.lock_change_animation_until_focus_out_or_QTE_done = false;
		}
		helper.PlayAnimation(PlayerAnimType::Attack_With_Broom);
		helper.lock_move = true;
		helper.lock_interact = true;
		helper.lock_attack = true;
		helper.lock_change_animation = true;
	}
}

void PlayerController::ShowStaminaUI()
{
	ui_stamina_background->SmoothAppear(0);
	ui_stamina_fill->SmoothAppear(0);
}

void PlayerController::AdjustStaminaUI()
{
	float stamina_ratio = stamina / max_stamina;

	UINT ori_size = (UINT)ui_stamina_fill_size.x;
	UINT fill_size = (UINT)(ori_size * stamina_ratio);
	
	int gap = ori_size - fill_size;

	if (gap % 2 == 1) gap += 1;
	ui_stamina_fill->anchor.x = -(gap / 2);

	//ui_stamina_fill->bubbles[0]->renderer->SetSize(fill_size, ui_stamina_fill_size.y);
	ui_stamina_fill->bubble_size.x = fill_size;
	ui_stamina_fill->bubbles[0]->target_size.x = fill_size;
	ui_stamina_fill->bubbles[0]->cur_size.x = fill_size;
	ui_stamina_fill->bubbles[0]->do_not_turn_off_when_scale_goto_zero = true;

	/*UINT anchor_x = 
	float anchor_x = -(ui_stamina_size.x * (1.f - stamina_ratio));*/
	//ui_stamina_fill->anchor.x = anchor_x;

	ui_stamina_fill->bubbles[0]->renderer->SetColor(Color::Lerp(stamina_fill_color_exhausted, stamina_fill_color_full, stamina_ratio));
}

void PlayerController::HideStaminaUI()
{
	ui_stamina_background->SmoothDisappear(0);
	ui_stamina_fill->SmoothDisappear(0);
}

//void PlayerController::Throw()
//{
//	auto& input = GameInputSystem::GetInstance();
//	if (interact_at_current_frame) return;
//
//	if (input.IsKeyDown(interact_key) && current_hold != nullptr)
//	{
//		current_hold->Throw(separated_interact_direction);
//		current_hold = nullptr;
//	}
//}

//void PlayerController::Cook()
//{
//	auto& input = GameInputSystem::GetInstance();
//
//	if (input.IsKeyDown(interact_key) && current_use_cooker != nullptr)
//	{
//		current_use_cooker->OnCook(this);
//	}
//}

//void PlayerController::TEST_CookDone()
//{
//	on_cooker = false;
//	current_use_cooker = nullptr;
//}

void PlayerController::Pick(HoldableType type, UINT sub_type)
{
	if (hand_graphic == nullptr) hand_graphic = FindHoldingObject(&transform);

	if (something_on_hand == true)
	{
		return;
	}

	bool result = hand_graphic->SetType(type, sub_type);
	if (result == false)
	{
		printf("Trying to pick undefined ingredient.");
		__debugbreak;
		return;
	}
	else
	{
		something_on_hand = true;
		hold_type = type;
		hold_subtype = sub_type;
		if (holdAudio) holdAudio->Play();
	}
}

std::pair<HoldableType, UINT> PlayerController::Swap(HoldableType type, UINT sub_type)
{
	if (hand_graphic == nullptr) hand_graphic = FindHoldingObject(&transform);
	if (something_on_hand == false) return std::pair<HoldableType, UINT>(HoldableType::None, 0);

	std::pair<HoldableType, UINT> own{ hold_type, hold_subtype };

	bool result = hand_graphic->SetType(type, sub_type);
	if (result == false)
	{
		printf("Trying to pick undefined ingredient.");
		__debugbreak;
		return std::pair<HoldableType, UINT>(HoldableType::None, 0);
	}
	else
	{
		if (dropAudio) dropAudio->Play();
		if (holdAudio) holdAudio->Play();
		hold_type = type;
		hold_subtype = sub_type;
	}

	return own;
}

void PlayerController::PutDown()
{
	hand_graphic->SetEmpty();
	something_on_hand = false;
	if (dropAudio) dropAudio->Play();
}

void PlayerController::InformQTEDone()
{
	helper.lock_change_animation_until_focus_out_or_QTE_done = false;
	helper.PlayAnimation(PlayerAnimType::Idle);
}

void PlayerController::PlayCookingAnimation(InteractableType type)
{
	switch (current_focus->type)
	{
	case InteractableType::Cooker_Cut:
		helper.PlayAnimation(PlayerAnimType::Chopping);
		helper.lock_change_animation_until_focus_out_or_QTE_done = true;
		break;
	case InteractableType::Cooker_Grind:
		helper.PlayAnimation(PlayerAnimType::Grinding);
		helper.lock_change_animation_until_focus_out_or_QTE_done = true;
		break;
	case InteractableType::Cooker_Squeeze:
		helper.PlayAnimation(PlayerAnimType::Squeezing);
		helper.lock_change_animation_until_focus_out_or_QTE_done = true;
		break;
	case InteractableType::Cauldron:
		helper.PlayAnimation(PlayerAnimType::BlowingBellows);
		helper.lock_change_animation_until_focus_out_or_QTE_done = true;
		break;
	}
}

void PlayerController::FindBroom(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"Broom"))
		{
			broom = c->gameObject.IsComponent<BroomComponent>();
		}
		if (broom != nullptr) return;
		else FindBroom(c);
	}
}

void PlayerController::FindKnife(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"Knife"))
		{
			knife = &c->gameObject;
		}
		if (knife != nullptr) return;
		else FindKnife(c);
	}
}

void PlayerController::FindGrinder(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"Grinder_P"))
		{
			grinder = &c->gameObject;
		}
		if (grinder != nullptr) return;
		else FindGrinder(c);
	}
}

Holding* PlayerController::FindHoldingObject(Transform* parent)
{
	Holding* holding{ nullptr };
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"HoldingObject"))
		{
			holding = c->gameObject.IsComponent<Holding>();
		}
		if (holding) return holding;
		else
		{
			holding = FindHoldingObject(c);
		}
	}
	return holding;
}

void PlayerController::FindAnimationObject(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (TransformAnimation* animator = c->gameObject.IsComponent<TransformAnimation>(); animator != nullptr)
		{
			player_animator = animator;
		}

		if (player_animator != nullptr) return;
		else
		{
			FindAnimationObject(c);
		}
	}
}

void PlayerController::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("CharacterController"))
	{
		float height = controller->GetHeight();
		float radius = controller->GetRadius();
		ImGui::Text("Collider Size : %f, %f", height, radius);
		ImGui::SliderFloat("Max Movement Speed", &max_speed, 0.0f, 100.0f);

		//ImGui::SliderFloat("Dash Speed Proportion", &dash_speed_proportion, 0.0f, 5.0f);
		//ImGui::SliderFloat("Dash Duration", &dash_duration, 0.0f, 0.5f);
		//ImGui::SliderFloat("Dash Deceleration", &dash_deceleration, 0.0f, 100.0f);
		//ImGui::SliderFloat("Dash Cooldown Time", &dash_cooldown, 0.0f, 2.0f);
		ImGui::DragFloat("Max Stamina", &max_stamina);
		ImGui::DragFloat("Dash Speed Mult", &dash_speed_increase_mult);
		ImGui::DragFloat("Dash Cost Per Second", &dash_consume_stamina_per_second);
		ImGui::DragFloat("Dash Recover Per Second", &stamina_recover_amount_per_second);

		ImGui::SliderFloat("Rotation Speed", &rotate_speed, 0.0f, 100.0f);
		ImGui::Checkbox("Smooth Rotation", &smooth_rotation);
		ImGui::Checkbox("Adjust speed by rotation", &fasten_rotation_speed_by_delta_angle);
		ImGui::Checkbox("Separate Interact Direction and Player Forward", &separate_interact_direction);
		if (ImGui::Button("Set Collider Size Automatically"))
		{
			controller->SetSizeAutomatically();
		}
		static float _temp[2]{ 0.0f, 0.0f };
		ImGui::InputFloat2("Set Size:", &_temp[0]);
		if (ImGui::Button("Set Collider Size Manually"))
		{
			if (_temp[0] <= 0.0f) _temp[0] = 0.1f;
			if (_temp[1] <= 0.0f) _temp[1] = 0.1f;
			controller->SetSize(_temp[0], _temp[1]);
		}
		ImGui::TreePop();
	}

	ImGui::InputText("VFX_Run", (char*)run.c_str(), run.size(), ImGuiInputTextFlags_CallbackResize,
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
		}, & run);

	ImGui::PopID();
}

void PlayerController::Serialized(std::ofstream& ofs)
{
	//버전 헤더
	constexpr uint32_t version = 2;
	Binary::Write::data<float>(ofs, FLT_MAX);
	Binary::Write::data<uint32_t>(ofs, version);
	Binary::Write::data(ofs, max_speed);
	if (version)
	{
		Binary::Write::data(ofs, max_stamina);
		Binary::Write::data(ofs, dash_speed_increase_mult);
		Binary::Write::data(ofs, dash_consume_stamina_per_second);
		Binary::Write::data(ofs, stamina_recover_amount_per_second);
	}
	Binary::Write::data(ofs, shot_ray_interval);
	Binary::Write::Vector3(ofs, separated_interact_direction);
	Binary::Write::data(ofs, superficial_rotation);
	Binary::Write::data(ofs, rotate_speed);
	Binary::Write::data(ofs, smooth_rotation);
	Binary::Write::data(ofs, fasten_rotation_speed_by_delta_angle);
	Binary::Write::data(ofs, separate_interact_direction);

	if (version >= 2)
	{
		Binary::Write::string(ofs, run);
	}
}

void PlayerController::Deserialized(std::ifstream& ifs)
{
	float header = Binary::Read::data<float>(ifs);
	uint32_t version = 0;
	if (header == FLT_MAX)
	{
		version = Binary::Read::data<uint32_t>(ifs);
		max_speed = Binary::Read::data<float>(ifs);
	}
	else
	{
		max_speed = header;
	}

	if (version > 0)
	{
		max_stamina = Binary::Read::data<float>(ifs);
		dash_speed_increase_mult = Binary::Read::data<float>(ifs);
		dash_consume_stamina_per_second = Binary::Read::data<float>(ifs);
		stamina_recover_amount_per_second = Binary::Read::data<float>(ifs);
	}
	shot_ray_interval = Binary::Read::data<float>(ifs);
	separated_interact_direction = Binary::Read::Vector3(ifs);
	superficial_rotation = Binary::Read::data<float>(ifs);
	rotate_speed = Binary::Read::data<float>(ifs);
	smooth_rotation = Binary::Read::data<bool>(ifs);
	fasten_rotation_speed_by_delta_angle = Binary::Read::data<bool>(ifs);
	separate_interact_direction = Binary::Read::data<bool>(ifs);

	if (version >= 2)
	{
		run = Binary::Read::string(ifs);
	}
}

