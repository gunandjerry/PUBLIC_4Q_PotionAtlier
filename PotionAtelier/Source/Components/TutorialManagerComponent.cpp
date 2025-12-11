#include "TutorialManagerComponent.h"
#include "Interactable.h"
#include "IngredientStand.h"
#include "PlayerController.h"
#include "StringResource.h"
#include "TextBubble.h"
#include "UIPoping.h"
#include "TextPoping.h"
#include "../ResourceFinder.h"
#include "Object/ButtonObject/GameStartButton.h"

#include "DeliverCounter.h"
#include "../Object/GameManager.h"

using namespace TimeSystem;

void TutorialManagerComponent::ListenEvent(TutorialEventType type)
{
	for (auto& event : events[type]) event();
	events[type].clear();
}

bool TutorialManagerComponent::CanInteract(Interactable* interactable)
{
	switch (interactable->type)
	{
	case InteractableType::None:
		return true;
	case InteractableType::Cooker_Cut:
		return !lock_interact_cutting_board;
	case InteractableType::Cooker_Grind:
		return !lock_interact_grinder;
	case InteractableType::Cooker_Squeeze:
		return !lock_interact_squeezer;
	case InteractableType::Counter:
		return !lock_interact_counter;
	case InteractableType::IngredientStand:
	{
		if (lock_interact_ingredient_stand == true) return false;
		else
		{
			IngredientStand* stand = static_cast<IngredientStand*>(interactable);
			switch (stand->GetIngredientType())
			{
			case IngredientType::DragonTail:
				return !lock_interact_ingredient_stand_type_dragon_tail;
			case IngredientType::MagicFlower:
				return !lock_interact_ingredient_stand_type_magic_flower;
			case IngredientType::MagicWood:
				return !lock_interact_ingredient_stand_type_magic_wood;
			}
			return false;
		}
	}
	case InteractableType::TrashBin:
		return !lock_interact_trash_bin;
	case InteractableType::EmptyTable:
		return !lock_interact_empty_table;
	case InteractableType::Cauldron:
		return !lock_interact_cauldron;
	}
}


void TutorialManagerComponent::Start()
{
	player = GameObject::Find(L"Player");
	player_controller = &player->GetComponent<PlayerController>();
	player_textbubble = &player->GetComponent<TextBubble>();

	upper_text_ui = &AddComponent<UIPoping>();
	upper_text_ui->Initialize(1, 0, upper_text_size, upper_text_anchor);
	upper_text_ui->SetTexture(0, ResourceFinder::GetTutorialBubbleImage());

	upper_text = &AddComponent<TextPoping>();
	upper_text->Initialize(upper_text_scale, upper_text_anchor + upper_text_additional_anchor);
	upper_text->renderer->SetColor({ 0, 0, 0, 1 });
	upper_text->renderer->drawSpeed = 1.0f;

	tuto_popup = &AddComponent<UIRenderComponenet>();
	tuto_popup->drawSpeed = 2.0f;
	tuto_popup->Enable = false;
	SIZE clientSize = D3D11_GameApp::GetClientSize();
	tuto_popup->SetTransform(clientSize.cx * 0.5f, clientSize.cy * 0.5f, tutorial_popup_size.x, tutorial_popup_size.y);

	if (player == nullptr || player_controller == nullptr || player_textbubble == nullptr) return;

	//StartTutorial();

	tutorial_37_close_position = transform.position;
}

void TutorialManagerComponent::Update()
{
	if (on_tutorial == false) return;

	if (process_tutorial_by_press_key)
	{
		auto& input = GameInputSystem::GetInstance();
		if (input.IsKeyDown(turn_text_bubble_key))
		{
			ProcessTutorial();
		}
	}



	// 카운터 펀치 위치에 가까이 다가갔을 때
	if (step == 9 && sub_step == 1)
	{
		Vector3 pPos = player->transform.GetPosition();
		pPos.y = 0.0f;
		Vector3 to = pPos - tutorial_37_close_position;
		if (to.LengthSquared() < tutorial_37_close_dist_threshold)
		{
			ListenEvent(TutorialEventType::GET_CLOSE_TO_ANGRY_COUNTER);
		}

		tutorial_37_close_dist_threshold += Time.DeltaTime;
	}
}

void TutorialManagerComponent::InspectorImguiDraw()
{

}

void TutorialManagerComponent::Serialized(std::ofstream& ofs)
{
	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 0;
	Binary::Write::data(ofs, header); //헤더
	Binary::Write::data(ofs, version); //버전
}

void TutorialManagerComponent::Deserialized(std::ifstream& ifs)
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
}

void TutorialManagerComponent::StartTutorial()
{
	on_tutorial = true;
	step = 0;
	sub_step = 0;
	std::wstring_view path = L"Resource/GameObject/TutorialHint.GameObject";
	tuto_popup_object = static_cast<UIMaterialObject*>(gameObjectFactory.DeserializedObject(path.data()));
  	tuto_popup_object->Active = false;

	step_funcs.clear();
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step1, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step2, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step3, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step4, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step5, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step6, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step7, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step8, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step9, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step10, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step11, this));
	step_funcs.push_back(std::bind(&TutorialManagerComponent::Step12, this));

	ProcessTutorial();
}

void TutorialManagerComponent::LockAll()
{
	lock_move = true;
	lock_interact = true;
	lock_interact_cutting_board = true;
	lock_interact_squeezer = true;
	lock_interact_grinder = true;
	lock_interact_counter = true;
	lock_interact_trash_bin = true;
	lock_interact_cauldron = true;
	lock_interact_empty_table = true;
	lock_interact_ingredient_stand = true;
	lock_interact_ingredient_stand_type_dragon_tail = true;
	lock_interact_ingredient_stand_type_magic_flower = true;
	lock_interact_ingredient_stand_type_magic_wood = true;
}

void TutorialManagerComponent::UnlockAll()
{
	lock_move = false;
	lock_interact = false;
	lock_interact_cutting_board = false;
	lock_interact_squeezer = false;
	lock_interact_grinder = false;
	lock_interact_counter = false;
	lock_interact_trash_bin = false;
	lock_interact_cauldron = false;
	lock_interact_empty_table = false;
	lock_interact_ingredient_stand = false;
	lock_interact_ingredient_stand_type_dragon_tail = false;
	lock_interact_ingredient_stand_type_magic_flower = false;
	lock_interact_ingredient_stand_type_magic_wood = false;
}

void TutorialManagerComponent::ProcessTutorial()
{
	step_funcs[step]();
}

void TutorialManagerComponent::Step1()
{
	if (sub_step == 0)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_1"));
		player_textbubble->ShowBubble();
		process_tutorial_by_press_key = true;
		LockAll();
	}
	else if (sub_step >= 1 && sub_step < 3)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(std::format(L"TT_{}", sub_step + 1)));
	}
	else if (sub_step == 3)
	{
		GameManager::GetGM().OrderPotion(PotionType::HealthPotion, [] {}, []{});

		// 주문서 출력
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_4"));
	}
	else if (sub_step == 4)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_5"));
	}
	else if (sub_step == 5)
	{
		player_textbubble->HideBubble();
		process_tutorial_by_press_key = false;

		SetUpperText(StringResource::GetTutorialText(L"TT_6"));
		ShowUpperText();

		lock_move = false;
		lock_interact = false;
		lock_interact_ingredient_stand = false;
		lock_interact_ingredient_stand_type_dragon_tail = false;

		events[TutorialEventType::GET_INGREDIENT_FROM_INGREDIENT_STAND].push_back([this]() {
			++step;
			sub_step = 0;

			process_tutorial_by_press_key = true;

			lock_move = true;
			lock_interact = true;
			lock_interact_ingredient_stand = true;
			lock_interact_ingredient_stand_type_dragon_tail = true;

			HideUpperText();
			player_textbubble->ShowBubble();

			ProcessTutorial();
																				  });
	}

	++sub_step;
}

void TutorialManagerComponent::Step2()
{
	if (sub_step == 0)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_7"));
	}
	else if (sub_step == 1)
	{
		player_textbubble->HideBubble();
		process_tutorial_by_press_key = false;

		SetUpperText(StringResource::GetTutorialText(L"TT_8"));
		ShowUpperText();

		lock_move = false;
		lock_interact = false;
		lock_interact_cutting_board = false;

		events[TutorialEventType::CUTTING_START].push_back([this]() {
			HideUpperText();

			lock_move = true;
			lock_interact = true;
			lock_interact_cutting_board = true;

			process_tutorial_by_press_key = true;

			tuto_popup->SetTexture(ResourceFinder::GetTutorialPopupImage(1));
			tuto_popup->Enable = true;
			
			++step;
			sub_step = 0;
																				  });
	}

	++sub_step;
}

void TutorialManagerComponent::Step3()
{
	if (sub_step == 0)
	{
		tuto_popup->Enable = false;

		lock_move = false;
		lock_interact = false;
		lock_interact_cutting_board = false;
		process_tutorial_by_press_key = false;

		events[TutorialEventType::CUTTING_DONE].push_back([this]() {
			lock_move = true;
			lock_interact = true;
			lock_interact_cutting_board = true;

			process_tutorial_by_press_key = true;

			player_textbubble->ShowBubble();
			player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_10"));

			++step;
			sub_step = 0;
														   });
	}

	++sub_step;
}

void TutorialManagerComponent::Step4()
{
	if (sub_step == 0)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_11"));
	}
	else if (sub_step == 1)
	{
		player_textbubble->HideBubble();

		lock_move = false;
		process_tutorial_by_press_key = false;

		SetUpperText(StringResource::GetTutorialText(L"TT_12"));
		ShowUpperText();

		events[TutorialEventType::DASH].push_back([this]() {
			Time.DelayedInvok(
				[this]() {
					lock_move = true;

					process_tutorial_by_press_key = true;

					HideUpperText();

					player_textbubble->ShowBubble();
					player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_13"));

					++step;
					sub_step = 0;
				},
				0.5f); // 대쉬 키 입력 n초 후
														  });
	}

	++sub_step;
}

void TutorialManagerComponent::Step5()
{
	if (sub_step == 0)
	{
		player_textbubble->HideBubble();

		lock_move = false;
		lock_interact = false;
		lock_interact_cauldron = false;

		process_tutorial_by_press_key = false;

		SetUpperText(StringResource::GetTutorialText(L"TT_14"));
		ShowUpperText();

		events[TutorialEventType::INPUT_INGREDIENT_INTO_CAULDRON].push_back([this]() {
			lock_move = true;
			lock_interact = true;
			lock_interact_cauldron = true;

			process_tutorial_by_press_key = true;

			HideUpperText();

			player_textbubble->ShowBubble();
			player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_15"));

			++step;
			sub_step = 0;
												  });
	}

	++sub_step;
}

void TutorialManagerComponent::Step6()
{
	if (sub_step == 0)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_16"));
	}
	else if (sub_step == 1)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_17"));
	}
	else if (sub_step == 2)
	{
		player_textbubble->HideBubble();

		lock_move = false;
		lock_interact = false;
		lock_interact_ingredient_stand = false;
		lock_interact_ingredient_stand_type_magic_flower = false;
		lock_interact_cauldron = false;

		process_tutorial_by_press_key = false;

		SetUpperText(StringResource::GetTutorialText(L"TT_18"));
		ShowUpperText();

		events[TutorialEventType::INPUT_INGREDIENT_INTO_CAULDRON].push_back([this]() {
			lock_move = true;
			lock_interact = true;
			lock_interact_ingredient_stand = true;
			lock_interact_ingredient_stand_type_magic_wood = true;
			lock_interact_cauldron = true;

			HideUpperText();

			player_textbubble->ShowBubble();
			player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_19"));

			events[TutorialEventType::BOILING_DONE].push_back([this]() {
				process_tutorial_by_press_key = true;

				player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_20"));

				++step;
				sub_step = 0;
																				});
																			});
	}

	++sub_step;
}

void TutorialManagerComponent::Step7()
{
	if (sub_step == 0)
	{
		player_textbubble->HideBubble();

		lock_move = false;
		lock_interact = false;
		lock_interact_cauldron = false;

		events[TutorialEventType::GET_POTION_FROM_CAULDRON].push_back([this]() {
			lock_move = true;
			lock_interact = true;
			lock_interact_cauldron = true;

			process_tutorial_by_press_key = true;

			player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_21"));
			player_textbubble->ShowBubble();

			++step;
			sub_step = 0;
														  });
	}
}

void TutorialManagerComponent::Step8()
{
	if (sub_step >= 0 && sub_step < 4)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(std::format(L"TT_{}", sub_step + 22)));
	}
	else if (sub_step == 4)
	{
		process_tutorial_by_press_key = false;
		player_textbubble->HideBubble();

		SetUpperText(StringResource::GetTutorialText(L"TT_25"));
		ShowUpperText();

		lock_move = false;
		lock_interact = false;
		lock_interact_counter = false;

		events[TutorialEventType::SUBMIT_POTION_VIA_COUNTER].push_back([this]() {
			lock_move = true;
			lock_interact = true;
			lock_interact_counter = true;

			process_tutorial_by_press_key = true;

			HideUpperText();

			player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_26"));
			player_textbubble->ShowBubble();

			++step;
			sub_step = 0;
																	  });
	}

	++sub_step;
}

void TutorialManagerComponent::Step9()
{
	if (sub_step == 0)
	{
		player_textbubble->HideBubble();

		tuto_popup->SetTexture(ResourceFinder::GetTutorialPopupImage(2));
		tuto_popup->Enable = true;
	}
	else if (sub_step == 1)
	{
		tuto_popup->Enable = false;

		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_28"));
		player_textbubble->ShowBubble();
	}
	else if (sub_step == 2)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_29"));
	}
	else if (sub_step == 3)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_30"));
	}
	else if (sub_step == 4)
	{
		player_textbubble->HideBubble();

		tuto_popup->SetTexture(ResourceFinder::GetTutorialPopupImage(3));
		tuto_popup->Enable = true;
	}
	else if (sub_step == 5)
	{
		tuto_popup->Enable = false;

		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_32"));
		player_textbubble->ShowBubble();
	}
	else if (sub_step == 6)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_33"));
	}
	else if (sub_step == 7)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_34"));
	}
	else if (sub_step == 8)
	{
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_35"));

		// 카운터 장난 전조 증상
		GameObject* counter = GameObject::Find(L"CounterObject");
		if (counter != nullptr)
		{
			counter->GetComponent<DeliverCounter>().Tutorial_ShowFace();
		}

		++step;
		sub_step = -1;
	}

	++sub_step;
}

void TutorialManagerComponent::Step10()
{
	// 카운터에 붙어있는 상태로 넘어왔는데 카운터에 가까이 가라??
	if (sub_step == 0)
	{
		player_textbubble->HideBubble();

		SetUpperText(StringResource::GetTutorialText(L"TT_36"));
		ShowUpperText();


		tuto_popup_object->Active = true;

		lock_move = false;
		process_tutorial_by_press_key = false;

		events[TutorialEventType::GET_CLOSE_TO_ANGRY_COUNTER].push_back([this]() {
			lock_move = true;

			// 카운터 펀치 애니메이션
			GameObject* counter = GameObject::Find(L"CounterObject");
			if (counter != nullptr)
			{
				counter->GetComponent<DeliverCounter>().Tutorial_PunchAndHideFace();
			}

			Time.DelayedInvok(
				[this]()
				{
					process_tutorial_by_press_key = true;

					HideUpperText();

					player_textbubble->ShowBubble();
					player_textbubble->SetBubbleText(StringResource::GetTutorialText(L"TT_37"));

					++step;
					sub_step = 0;
				}, 0.7f);
																	   });
	}

	++sub_step;
}

void TutorialManagerComponent::Step11()
{
	if (sub_step >= 0 && sub_step < 3)
	{
		tuto_popup_object->Active = false;
		player_textbubble->SetBubbleText(StringResource::GetTutorialText(std::format(L"TT_{}", sub_step + 38)));
	}
	else if (sub_step == 3)
	{
		player_textbubble->HideBubble();

		tuto_popup->SetTexture(ResourceFinder::GetTutorialPopupImage(4));
		tuto_popup->Enable = true;
	}
	else if (sub_step == 4)
	{
		tuto_popup->Enable = false;
		UnlockAll();
		on_tutorial = false;
		auto event = GameObject::Find<SceneEventObject>(L"SceneEventObject");
		if (event)
		{
			auto ui = event->componenet->UI;
			if (ui)
			{
				ui->SetPosX(0);
				ui->SetPosY(0);
			}
			event->componenet->isReverse = false;
			event->componenet->EventStart();
		}
		Time.DelayedInvok(
			[]()
			{
				GameManager::GetGM().StageLoad(1);
			},
			1.0f
		);
	}

	++sub_step;
}

void TutorialManagerComponent::Step12()
{

}







void TutorialManagerComponent::ShowUpperText()
{
	upper_text->SmoothAppear();
	upper_text_ui->SmoothAppear(0);
}

void TutorialManagerComponent::HideUpperText()
{
	upper_text->SmoothDisappear();
	upper_text_ui->SmoothDisappear(0);
}

void TutorialManagerComponent::SetUpperText(std::wstring_view text)
{
	upper_text->SetText(text);
}
