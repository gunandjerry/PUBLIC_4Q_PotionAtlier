#include "TutorialButtonComponent.h"
#include "ResourceFinder.h"

void TutorialButtonComponent::Start()
{
	SIZE clientSize = D3D11_GameApp::GetClientSize();
	for (int i = 0; i < 5; ++i)
	{
		auto* ui = &AddComponent<UIRenderComponenet>();
		ui->SetTexture(ResourceFinder::GetTutoButtonImage(i + 1));
		ui->SetTransform(clientSize.cx * 0.5f, clientSize.cy * 0.5f, 1360, 815);
		ui->Enable = false;
		uis.push_back(ui);
	}

	event = &GetComponent<EventListener>();
	event->SetOnClickDown([this]() {
		Show();
	});
}

void TutorialButtonComponent::Update()
{
	auto& input = GameInputSystem::GetInstance();
	if (input.IsKeyDown(KeyboardKeys::Space))
	{
		if (show == true)
		{
			HidePage(show_idx);
			++show_idx;
			if (show_idx < uis.size())
			{
				ShowPage(show_idx);
			}
			else
			{
				show = false;
			}
		}
	}
}

void TutorialButtonComponent::Show()
{
	if (show == true) return;

	show_idx = 0;
	show = true;
	ShowPage(show_idx);
}

void TutorialButtonComponent::ShowPage(int idx)
{
	// 대충 등장 효과
	uis[idx]->Enable = true;
}

void TutorialButtonComponent::HidePage(int idx)
{
	// 대충 최장 효과
	uis[idx]->Enable = false;
}
