#include "Textbubble.h"
#include "TextPoping.h"
#include "UIPoping.h"
#include "../ResourceFinder.h"
#include "../StringResource.h"

float TextBubble::text_bubble_duration_common{ 2.5f };
float TextBubble::text_bubble_duration_punched{ 1.0f };


void TextBubble::Awake()
{
}

void TextBubble::Start()
{
	text = &AddComponent<TextPoping>();
	ui = &AddComponent<UIPoping>();

	if (type == TextBubbleType::Player)
	{
		background_size = { 340, 144 };
		anchor = { 0, -135 };
		text_additional_anchor = { 0, -0 };
		font_scale = 0.28f;
		font_color = { 0, 0, 0, 1 };

		space_bar_ui = &AddComponent<UIPoping>();
		space_bar_ui->Initialize(1, 0, space_bar_ui_size, anchor + space_bar_ui_additional_anchor);

		space_bar_ui->SetTexture(0, L"./Resource/UI/IconUI_Space.png");
		space_bar_ui->bubbles[0]->renderer->drawSpeed = -9.5f;


		ui->Initialize(1, 0, background_size, anchor);
		ui->SetTexture(0, ResourceFinder::GetTextBubbleImage(true));
		ui->bubbles[0]->renderer->drawSpeed = -10.f;
		text->Initialize(font_scale, anchor + text_additional_anchor); // TextBubble 이미지의 중점은 텍스트 출력 영역보다 살짝 아래임
		text->renderer->drawSpeed = -9.f;
	}
	else if (type == TextBubbleType::Gnome)
	{
		background_size = { 176, 81 };
		anchor = { 0, -120 };
		text_additional_anchor = { 0, -7 };
		font_scale = 0.24f;
		font_color = { 0, 0, 0, 1 };

		ui->Initialize(1, 0, background_size, anchor);
		ui->SetTexture(0, ResourceFinder::GetTextBubbleImage(false));
		ui->bubbles[0]->renderer->drawSpeed = -10.f;
		text->Initialize(font_scale, anchor + text_additional_anchor); // TextBubble 이미지의 중점은 텍스트 출력 영역보다 살짝 아래임
		text->renderer->drawSpeed = -9.f;
	}

	
	text->SetText(L"");

	text->renderer->SetColor(font_color);
	text->renderer->SetScale(font_scale);
	text->anchor = anchor + text_additional_anchor;
	text->base_size = font_scale;
}

void TextBubble::Update()
{
}


void TextBubble::ShowBubble()
{
	text->SmoothAppear();
	ui->SmoothAppear(0);

	if (type == TextBubbleType::Player)
	{
		space_bar_ui->SmoothAppear(0);
	}
}

void TextBubble::HideBubble()
{
	text->SmoothDisappear();
	ui->SmoothDisappear(0);

	if (type == TextBubbleType::Player)
	{
		space_bar_ui->SmoothDisappear(0);
	}
}

void TextBubble::SetBubbleText(std::wstring_view _text)
{
	text->SetText(_text);
}

void TextBubble::SetDrawSpeed(float speed)
{
	ui->bubbles[0]->renderer->drawSpeed = speed;
	text->renderer->drawSpeed = speed + 0.1f;
}

void TextBubble::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("TextBubble"))
	{
		if (ImGui::DragFloat("Font Scale", &font_scale))
		{
			text->renderer->SetScale(font_scale);
			text->base_size = font_scale;
		}
		if (ImGui::DragFloat("Font Anchor Y", &text_additional_anchor.y))
		{

		}


		if (ImGui::Button("Show"))
		{
			ShowBubble();
		}
		if (ImGui::Button("Hide"))
		{
			HideBubble();
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}