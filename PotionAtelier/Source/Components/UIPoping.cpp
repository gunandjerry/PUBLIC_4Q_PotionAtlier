#include "UIPoping.h"

using namespace TimeSystem;

UIBubble::UIBubble(UIRenderComponenet* renderer)
{
	if (renderer == nullptr) __debugbreak;

	this->renderer = renderer;
}

void UIBubble::UpdateTransform()
{
	t += Time.DeltaTime * poping_speed;

	if (t >= 1.0f)
	{
		cur_size = target_size;
		renderer->SetTransform(position.x, position.y, cur_size.x, cur_size.y);
		if (target_size.x == 0 && do_not_turn_off_when_scale_goto_zero == false) renderer->Enable = false;
		on_change = false;
	}
	else
	{
		cur_size.x = std::lerp(init_size.x, target_size.x, t);
		cur_size.y = std::lerp(init_size.y, target_size.y, t);

		renderer->SetTransform(position.x, position.y, cur_size.x, cur_size.y);
	}
}





void UIPoping::Update()
{
	UpdatePosition();


	/*auto& input = GameInputSystem::GetInstance();
	if (input.IsKeyDown(KeyboardKeys::J))
	{
		for (int i = 0; i < bubbles.size(); ++i)
		{
			SetTexture(i, L"./Resource/Sample/dragontail.png");
			SmoothAppear(i);
		}
	}
	if (input.IsKeyDown(KeyboardKeys::K))
	{
		for (int i = 0; i < bubbles.size(); ++i)
		{
			SmoothDisappear(i);
		}
	}*/
}

void UIPoping::UpdatePosition()
{
	Vector2 screen_point = Camera::GetMainCamera()->WorldToScreenPoint(transform.position) + anchor;
	float pos_x;
	if (bubble_num % 2 == 0)
	{
		pos_x = screen_point.x - ((bubble_num / 2 - 0.5f) * bubble_gap);
	}
	else
	{
		pos_x = screen_point.x - (bubble_num / 2 * bubble_gap);
	}

	for (int i = 0; i < bubbles.size(); ++i)
	{
		bubbles[i]->position = { pos_x, screen_point.y };
		if (bubbles[i]->renderer->Enable == true) bubbles[i]->UpdateTransform();

		pos_x += bubble_gap;
	}
}

void UIPoping::Initialize(int bubble_num, int bubble_gap, Vector2 bubble_size, Vector2 anchor)
{
	if (initialized)
	{
		__debugbreak;
	}

	this->bubble_num = bubble_num;
	this->bubble_gap = bubble_gap;
	this->bubble_size = bubble_size;
	this->anchor = anchor;

	for (int i = 0; i < bubble_num; ++i)
	{
		UIRenderComponenet& comp = AddComponent<UIRenderComponenet>();
		comp.Enable = false;
		
		UIBubble* bubble = new UIBubble(&comp);
		bubbles.push_back(bubble);
	}

	initialized = true;
}

void UIPoping::SetTexture(int i, std::wstring_view path)
{
	if (i >= bubble_num) return;

	bubbles[i]->renderer->SetTexture(path);
}

void UIPoping::SmoothAppear(int i)
{
	bubbles[i]->on_change = true;
	bubbles[i]->renderer->Enable = true;
	bubbles[i]->target_size = bubble_size;
	bubbles[i]->init_size = bubbles[i]->cur_size;
	bubbles[i]->t = 0.0f;
	UpdatePosition();
}

void UIPoping::SmoothDisappear(int i)
{
	bubbles[i]->on_change = true;
	bubbles[i]->renderer->Enable = true;
	bubbles[i]->target_size = { 0, 0 };
	bubbles[i]->init_size = bubbles[i]->cur_size;
	bubbles[i]->t = 0.0f;
	UpdatePosition();
}

void UIPoping::InstantAppear(int i)
{
	bubbles[i]->on_change = true;
	bubbles[i]->renderer->Enable = true;
	bubbles[i]->target_size = bubble_size;
	bubbles[i]->init_size = bubbles[i]->cur_size;
	bubbles[i]->t = 1.0f;
	UpdatePosition();
}

void UIPoping::InstantDisappear(int i)
{
	bubbles[i]->on_change = true;
	bubbles[i]->renderer->Enable = false;
	bubbles[i]->target_size = { 0, 0 };
	bubbles[i]->init_size = bubbles[i]->cur_size;
	bubbles[i]->t = 1.0f;
	UpdatePosition();
}
