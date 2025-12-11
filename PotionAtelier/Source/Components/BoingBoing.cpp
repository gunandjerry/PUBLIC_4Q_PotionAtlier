#include "BoingBoing.h"
#include "Component/Render/UIRenderComponenet.h"
#include "Object\AudioPlayerObject.h"

using namespace TimeSystem;

void BoingBoing::Awake()
{
	BoingPath default_boing;
	default_boing.boing_name = "SampleBoing";
	default_boing.paths = {
		{ 1.f, 1.f, 1.f }, { 0.6f, 0.6f, 0.6f }, { 0.8f, 1.4f, 0.8f }, { 1.2f, 0.8f, 1.2f }, { 1.f, 1.f, 1.f }
	};
	boings.push_back(std::move(default_boing));

	BoingPath default_boing2;
	default_boing2.boing_name = "SampleBoing2";
	default_boing2.paths = {
		{1.f, 1.f, 1.f}, {1.4f, 1.4f, 1.4f}, {0.8f, 0.8f, 0.8f}, {1.2f, 1.2f, 1.2f}, {1.f, 1.f, 1.f}
	};
	boings.push_back(std::move(default_boing2));
}

void BoingBoing::Start()
{
	if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Boing"))
	{
		SFX_Boing = findItem->IsComponent<AudioBankClip>();
	}

}

void BoingBoing::Update()
{
	if (current_boing != nullptr)
	{
		t += Time.DeltaTime * boing_speed;

		while (t >= 1.0f)
		{
			t -= 1.0f;
			boing_idx++;
		}

		if (boing_idx + 1 >= current_boing->paths.size())
		{
			transform.SetScale(original_scale * current_boing->paths.back());
			//for (auto& p : targets)
			//{
			//	// 원상복구
			//	p.first->transform.SetScale(p.second);
			//}
			t = 0.0f;
			boing_idx = 0;
			current_boing = nullptr;
		}
		else
		{
			transform.SetScale(original_scale * Vector3::Lerp(current_boing->paths[boing_idx], current_boing->paths[boing_idx + 1], t));
			/*for (auto& p : targets)
			{
				p.first->transform.SetScale(p.second * Vector3::Lerp(current_boing->paths[boing_idx], current_boing->paths[boing_idx + 1], t));
			}*/
		}
	}
}

void BoingBoing::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("Boing Boing"))
	{
		for (int i = 0; i < boings.size(); ++i)
		{
			if (ImGui::Button(boings[i].boing_name.c_str()))
			{
				Boing(i);
			}
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void BoingBoing::Boing(int idx)
{
	if (current_boing != nullptr) return;

	current_boing = &boings[idx];
	original_scale = transform.GetScale();
	//targets.clear();
	if (SFX_Boing) SFX_Boing->Play();
	//FindMeshesRecursively(&transform);
}

//void BoingBoing::FindMeshesRecursively(Transform* parent)
//{
//	int cn = parent->GetChildCount();
//	for (int i = 0; i < cn; ++i)
//	{
//		GameObject& child = parent->GetChild(i)->gameObject;
//		auto* renderer = child.IsComponent<SimpleMeshRender>();
//		if (renderer != nullptr)
//		{
//			targets.push_back(std::make_pair(&child, child.transform.GetScale()));
//		}
//		else
//		{
//			FindMeshesRecursively(&child.transform);
//		}
//	}
//}





void BoingBoingUI::Awake()
{
	BoingUIPath default_boing;
	default_boing.boing_name = "SampleBoing";
	default_boing.paths = {
		{ 1.f, 1.f }, { 0.6f, 0.6f }, { 0.8f, 1.4f }, { 1.2f, 0.8f }, { 1.f, 1.f }
	};
	boings.push_back(std::move(default_boing));

	BoingUIPath default_boing2;
	default_boing2.boing_name = "SampleBoing2";
	default_boing2.paths = {
		{1.f, 1.f}, {1.4f, 1.4f}, {0.8f, 0.8f}, {1.2f, 1.2f}, {1.f, 1.f}
	};
	boings.push_back(std::move(default_boing2));
}

void BoingBoingUI::Start()
{
	if (use_on_ui_render2 == false)
	{
		ui_render1 = IsComponent<UIRenderComponenet>();
	}
	else
	{
		ui_render2 = IsComponent<UIRenderComponenet2>();
	}
}

void BoingBoingUI::Update()
{
	if (current_boing != nullptr)
	{
		t += Time.DeltaTime * boing_speed;

		while (t >= 1.0f)
		{
			t -= 1.0f;
			boing_idx++;
		}

		if (boing_idx + 1 >= current_boing->paths.size())
		{
			if (use_on_ui_render2 == false)
			{
				if (ui_render1 != nullptr)
				{
					Vector2 new_size = original_scale * current_boing->paths.back();
					ui_render1->SetSize((size_t)new_size.x, (size_t)new_size.y);
				}
			}
			else
			{
				if (ui_render2 != nullptr)
				{
					Vector3 mult_scale = { current_boing->paths.back().x, current_boing->paths.back().y, 1 };
					Vector3 ori_scale = { original_scale.x, original_scale.y, 1 };
					ui_render2->transform.SetScale(ori_scale * mult_scale);
				}
			}

			t = 0.0f;
			boing_idx = 0;
			current_boing = nullptr;
		}
		else
		{
			if (use_on_ui_render2 == false)
			{
				if (ui_render1 != nullptr)
				{
					Vector2 new_size = original_scale * Vector2::Lerp(current_boing->paths[boing_idx], current_boing->paths[boing_idx + 1], t);
					ui_render1->SetSize((size_t)new_size.x, (size_t)new_size.y);
				}
			}
			else
			{
				if (ui_render2 != nullptr)
				{
					Vector2 new_size = original_scale * Vector2::Lerp(current_boing->paths[boing_idx], current_boing->paths[boing_idx + 1], t);
					ui_render2->SetWidth(new_size.x);
					ui_render2->SetHeight(new_size.y);
				}
			}
		}
	}
}

void BoingBoingUI::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("Boing Boing UI"))
	{
		for (int i = 0; i < boings.size(); ++i)
		{
			if (ImGui::Button(boings[i].boing_name.c_str()))
			{
				Boing(i);
			}
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void BoingBoingUI::Boing(int idx)
{
	if (current_boing != nullptr) return;

	current_boing = &boings[idx];

	if (use_on_ui_render2 == false)
	{
		original_scale = ui_render1->GetSize();
	}
	else
	{
		original_scale = { ui_render2->GetWidth(), ui_render2->GetHeight() };
	}
}