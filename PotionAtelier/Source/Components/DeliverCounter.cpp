#include "DeliverCounter.h"
#include "PlayerController.h"
#include "../Object/TutorialManager.h"
#include "../Object/GameManager.h"
#include "TextPoping.h"
#include "CustomerAI.h"
#include "FlyingGnome.h"
#include "../Object/GnomeCustomer.h"
#include "Object\AudioPlayerObject.h"
#include "CustomerSpawner.h"
#include "TextBubble.h"
#include "StringResource.h"

using namespace TimeSystem;

DeliverCounter::DeliverCounter()
{
	type = InteractableType::Counter;
}

void DeliverCounter::Awake()
{
	LoadVFX();
}

void DeliverCounter::Start()
{
	std::wstring wstr = std::wstring(serve.begin(), serve.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_Serve = findItem->IsComponent<ParticleSpawnComponent>();
	}


	earning_ui = &AddComponent<TextPoping>();
	earning_ui->Initialize(0.5f, start_ui_anchor);
	earning_ui->renderer->SetType(FontType::HangameSemiBold);

	FindFaceQuad(&transform);
	FindAnimator(&transform);

	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Laughter"))
	{
		SFX_Laughter = findItem->IsComponent<AudioBankClip>();
	}
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Drawer"))
	{
		SFX_Drawer = findItem->IsComponent<AudioBankClip>();
	}
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_GetHit"))
	{
		SFX_GetHit = findItem->IsComponent<AudioBankClip>();
	}
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Fly"))
	{
		SFX_Fly = findItem->IsComponent<AudioBankClip>();
	}
}

void DeliverCounter::Update()
{
	// 맞으면 대충 Tremble() 호출해주고 face_step = 5로 만들어서 Punch() 호출 스킵하고 얼굴 사라지게 하면 됨.
	if (face_step == 1) // smooth appear
	{
		show_face_t += Time.DeltaTime / show_face_time;

		if (show_face_t >= 1.0f)
		{
			face_quad->materialAsset.customData.SetField("alpha", 1.0f);
			show_face_t = 0.0f;
			++face_step;

			TrembleLoop();
		}
		else
		{
			face_quad->materialAsset.customData.SetField("alpha", show_face_t);
		}
	}
	else if (face_step == 2) // animation ready
	{
		show_face_t += Time.DeltaTime / (face_additional_maintain_time * 0.5f);

		if (show_face_t >= 1.0f)
		{
			show_face_t = 0.0f;
			++face_step;
			if (SFX_Laughter) SFX_Laughter->Play();
		}
		else
		{

		}
	}
	else if (face_step == 3) // face animation
	{
		show_face_t += Time.DeltaTime / face_animation_time;

		if (show_face_t >= 0.95f) // 1.0f으로 하면 첫 프레임으로 돌아감;
		{
			face_quad->materialAsset.customData.SetField("time_t", 0.95f);
			show_face_t = 0.0f;
			++face_step;
			punch = false;

			if (calm_down_it_just_a_kind_of_joke)
			{
				face_step = 100;
			}
		}
		else
		{
			face_quad->materialAsset.customData.SetField("time_t", show_face_t);
		}
	}
	else if (face_step == 4) // disappear ready
	{
		show_face_t += Time.DeltaTime / (face_additional_maintain_time * 0.5f);

		if (punch == false && show_face_t >= punch_point_t)
		{
			punch = true;
			Punch();
		}
		else if (show_face_t >= 1.0f)
		{
			show_face_t = 0.0f;
			++face_step;
		}
		else
		{

		}
	}
	else if (face_step == 5) // smooth disappear
	{
		show_face_t += Time.DeltaTime / show_face_time;

		if (show_face_t >= 1.0f)
		{
			face_quad->materialAsset.customData.SetField("alpha", 0.0f);
			face_quad->materialAsset.customData.SetField("time_t", 0.0f);
			show_face_t = 0.0f;
			face_step = 0;
		}
		else
		{
			face_quad->materialAsset.customData.SetField("alpha", 1 - show_face_t);
		}
	}


	// 얻는 돈 ui
	if (ui_show)
	{
		earning_t += Time.DeltaTime * ui_speed;
		if (earning_t >= 1.0f)
		{
			earning_ui->InstantDisappear();
		}
		else
		{
			earning_ui->anchor = Vector2::Lerp(start_ui_anchor, goal_ui_anchor, earning_t);
			earning_ui->renderer->SetColor(Color::Lerp(init_color, goal_color, earning_t));
		}
	}
}

void DeliverCounter::OnFocusIn(PlayerController* controller)
{
	//transform.SetScale({ 1.1f, 1.1f, 1.1f });
}

void DeliverCounter::OnFocusOut(PlayerController* controller)
{
	//transform.SetScale({ 1.0f, 1.0f, 1.0f });
}

bool DeliverCounter::OnInteract(PlayerController* controller)
{
	if (controller->something_on_hand == false) return false;
	if (controller->hold_type != HoldableType::Potion) return false;

	bool success = (GameManager::GetGM().ServeOrder((PotionType)controller->hold_subtype) != -1);

	TutorialManagerComponent* tm = TutorialManager::GetInstance();
	if (tm->OnTutorial() == true) tm->ListenEvent(TutorialEventType::SUBMIT_POTION_VIA_COUNTER);

	if (VFX_Serve) VFX_Serve->CreateParticle();


	int earning = GameManager::GetGM().GetPotionScore((PotionType)controller->hold_subtype);
	if (success && earning > 0)
	{
		earning_ui->SmoothAppear();
		ui_show = true;
		earning_t = 0.0f;
		if (GameManager::GetGM().IsFever())
			earning_ui->SetText(std::format(L"+ {} X2", earning));
		else
			earning_ui->SetText(std::format(L"+ {}", earning));
	}

	controller->PutDown();

	return true;
}

void DeliverCounter::OnAttacked(PlayerController* controller)
{
	Time.DelayedInvok([this]()
	{
		if ((face_step >= 1 && face_step < 4) || (face_step == 4 && show_face_t < punch_point_t))
		{
			face_step = 5;
			show_face_t = 0.0f;
			StopTremble();
		}
	}, 0.5f);
}

void DeliverCounter::ShowFace()
{
	if (face_step > 0) return;

	face_step = 1;
	show_face_t = 0.0f;
	punch = false;
}

void DeliverCounter::StopTremble()
{
	animator->StopClip(true);
}

void DeliverCounter::TrembleLoop()
{
	animator->PlayClip(L"Armature.001|Armature.001|Armature.001|Armature.001|Armature|precursor", true);
}

void DeliverCounter::Punch()
{
	if (SFX_Drawer) SFX_Drawer->Play();
	if (SFX_GetHit) SFX_GetHit->Play();
	if (SFX_Fly) SFX_Fly->Play();
	animator->PlayClip(L"Armature.001|Armature.001|Armature.001|Armature.001|Armature|punch.003", false);

	Time.DelayedInvok([=]() {
		auto* target = CustomerSpawner::FindFirstOrderGnome();
		if (target != nullptr)
		{
			target->GetComponent<FlyingGnome>().FlyMeToTheMoon();

			// 버그있는듯?
			/*auto& tb = target->GetComponent<TextBubble>();
			tb.ShowBubble();
			int i = Random::Range(16, 18);
			tb.SetBubbleText(StringResource::GetTutorialText(std::format(L"CT_{}", i)));

			TimeSystem::Time.DelayedInvok([&tb]()
			{
				tb.HideBubble();
			}, TextBubble::text_bubble_duration_punched);*/

			if (target->GetComponent<CustomerAI>().GetCurrentState() == CustomerStateType::Order)
			{
				GameManager::GetGM().OrderDeleteFront();
			}
		}
					  }, 0.35f);
}

void DeliverCounter::Tutorial_ShowFace()
{
	if (face_step > 0) return;

	face_step = 1;
	show_face_t = 0.0f;
	punch = false;
	calm_down_it_just_a_kind_of_joke = true;
}

void DeliverCounter::Tutorial_PunchAndHideFace()
{
	Punch();
	face_step = 5;
}

void DeliverCounter::FindFaceQuad(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"FaceQuad"))
		{
			face_quad = &c->gameObject.GetComponent<PBRMeshRender>();
		}
		if (face_quad != nullptr) return;
		else FindFaceQuad(c);
	}
}

void DeliverCounter::FindAnimator(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (TransformAnimation* _animator = c->gameObject.IsComponent<TransformAnimation>(); _animator != nullptr)
		{
			animator = _animator;
		}

		if (animator != nullptr) return;
		else
		{
			FindAnimator(c);
		}
	}
}

void DeliverCounter::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	ImGui::InputText("VFX_Serve", (char*)serve.c_str(), serve.size(), ImGuiInputTextFlags_CallbackResize,
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
		}, &serve);
	if (ImGui::Button("Save"))
	{
		SaveVFX();
	}

	if (ImGui::TreeNode("Counter"))
	{
		ImGui::Text("Warning! Only use after start.");
		if (ImGui::Button("Show Face"))
		{
			ShowFace();
		}
		if (ImGui::Button("Punch"))
		{
			Punch();
		}
		if (ImGui::Button("Tremble"))
		{
			StopTremble();
		}
		if (ImGui::Button("OnAttacked"))
		{
			OnAttacked(nullptr);
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void DeliverCounter::SaveVFX()
{
	using namespace Binary;
	if (!std::filesystem::exists(VFX_ServeDataPath))
	{
		std::filesystem::create_directories(std::filesystem::path(VFX_ServeDataPath).parent_path());
	}
	std::ofstream ofs(VFX_ServeDataPath, std::ios::binary | std::ios::trunc);
	{
		constexpr size_t Version = 0;
		Write::data(ofs, Version);

		Write::string(ofs, serve);
	}
	ofs.close();
}

void DeliverCounter::LoadVFX()
{
	using namespace Binary;
	std::ifstream ifs(VFX_ServeDataPath, std::ios::binary);
	if (ifs.is_open())
	{
		size_t Version = 0;
		Version = Read::data<size_t>(ifs);

		if (Version == 0)
		{
			serve = Read::string(ifs);
		}
	}
	ifs.close();
}

