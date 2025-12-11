#pragma once
#include "framework.h"

enum class TextBubbleType
{
	Player,
	Gnome
};

class TextBubble : public Component
{
	friend class TutorialManagerComponent;

	class TextPoping* text{ nullptr };
	class UIPoping* ui{ nullptr };

	Vector2 background_size{ 340, 144 };
	Vector2 anchor{ 0, -100 };
	Vector2 text_additional_anchor{ 0, -20 };
	float font_scale{ 0.4f };
	Color font_color{ 0, 0, 0, 1 };

	// 스페이스바 UI
	Vector2 space_bar_ui_size{ 107, 47 };
	Vector2 space_bar_ui_additional_anchor{ 134, 46 };
	UIPoping* space_bar_ui{ nullptr };
public:
	TextBubbleType type{ TextBubbleType::Player };

	static float text_bubble_duration_common;
	static float text_bubble_duration_punched;


	virtual void Awake();
	virtual void Start();

protected:
	virtual void Update();
	virtual void FixedUpdate() {}
	virtual void LateUpdate() {}


public:
	void ShowBubble();
	void HideBubble();
	void SetBubbleText(std::wstring_view text);
	void SetDrawSpeed(float speed);


public:
	virtual void InspectorImguiDraw() override;
};