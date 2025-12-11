#pragma once
#include "framework.h"

class TextPoping : public Component
{
	friend class TextBubble;

	bool initialized{ false };

	bool on_change{ false }; // obsolete
	Vector2 position{ 0, 0 };
	float target_size{ 0 };
	float init_size{ 0 };
	float cur_size{ 0 };
	
	float base_size{ 0 };
	
	float t{ 0.0f };
	float poping_speed{ 12.0f };

public:
	TextRender* renderer{ nullptr };
	Vector2 anchor{ 0, 0 }; // px


	~TextPoping();
	virtual void Awake() {}
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}


private:
	void UpdatePosition();


public:
	void Initialize(float size, Vector2 anchor);
	void SetText(std::wstring_view text);
	void ChangeSize(float size) { base_size = size; }

	void SmoothAppear();
	void SmoothDisappear();
	void InstantAppear();
	void InstantDisappear();
};

