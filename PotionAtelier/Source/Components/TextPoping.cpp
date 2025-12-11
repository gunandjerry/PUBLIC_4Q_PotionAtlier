#include "TextPoping.h"

using namespace TimeSystem;

TextPoping::~TextPoping() = default;

void TextPoping::Update()
{
	UpdatePosition();
}

void TextPoping::UpdatePosition()
{
	Vector2 screen_pos = Camera::GetMainCamera()->WorldToScreenPoint(transform.GetPosition()) + anchor;
	SIZE client_size = D3D11_GameApp::GetClientSize();

	Vector2 ndc_pos;
	ndc_pos.x = ((screen_pos.x / client_size.cx) - 0.5f) * 2.0f;
	ndc_pos.y = (1.0f - (screen_pos.y / client_size.cy) - 0.5f) * 2.0f;
	renderer->transform.SetPosition({ ndc_pos.x, ndc_pos.y, 0 });

	t += Time.DeltaTime * poping_speed;

	if (t >= 1.0f)
	{
		cur_size = target_size;
		renderer->SetScale(cur_size);
		if (target_size == 0.0f)
		{
			renderer->Enable = false;
		}
		on_change = false;
	}
	else
	{
		cur_size = std::lerp(init_size, target_size, t);
		renderer->SetScale(cur_size);
	}
}

void TextPoping::Initialize(float size, Vector2 anchor)
{
	if (initialized)
	{
		__debugbreak;
	}

	GameObject* rendererObj = NewGameObject<GameObject>(L"TextBubbleObj");
	// 텍스트 렌더러는 부모 포지션을 상속하므로 부모 설정 X
	//rendererObj->transform.SetParent(transform);
	renderer = &rendererObj->AddComponent<TextRender>();
	renderer->Enable = false;

	base_size = size;
	this->anchor = anchor;

	initialized = true;
}

void TextPoping::SetText(std::wstring_view text)
{
	renderer->SetText(text.data());
}

void TextPoping::SmoothAppear()
{
	on_change = true;
	renderer->Enable = true;
	target_size = base_size;
	init_size = cur_size;
	t = 0.0f;
	UpdatePosition();
}

void TextPoping::SmoothDisappear()
{
	on_change = true;
	renderer->Enable = true;
	target_size = 0.0f;
	init_size = cur_size;
	t = 0.0f;
	UpdatePosition();
}

void TextPoping::InstantAppear()
{
	on_change = true;
	renderer->Enable = true;
	target_size = base_size;
	init_size = cur_size;
	t = 1.0f;
	UpdatePosition();
}

void TextPoping::InstantDisappear()
{
	on_change = true;
	renderer->Enable = false;
	target_size = 0.0f;
	init_size = cur_size;
	t = 1.0f;
	UpdatePosition();
}
