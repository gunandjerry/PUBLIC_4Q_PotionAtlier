#pragma once
#include <vector>
#include "framework.h"

// 특정 오브젝트 상대 위치에 부드럽게 UI 버블 띄우기

struct UIBubble
{
	class UIRenderComponenet* renderer{ nullptr };
	Vector2 position{ 0, 0 };
	Vector2 target_size{ 0, 0 };
	Vector2 init_size{ 0, 0 };
	Vector2 cur_size{ 0, 0 };
	
	bool do_not_turn_off_when_scale_goto_zero{ false };
	bool on_change{ false }; // obsolete
	float t{ 0.0f };
	float poping_speed = 12.0f;

	UIBubble(class UIRenderComponenet* renderer);

	void UpdateTransform();
};

class UIPoping : public Component
{
	friend class PlayerController;
	friend class Cauldron;

	bool initialized{ false };

	int bubble_num{ 0 };
	int bubble_gap{ 0 };
	Vector2 bubble_size{ 0, 0 }; // px



public:
	std::vector<UIBubble*> bubbles;
	Vector2 anchor{ 0, 0 }; // px

	virtual void Awake() {}
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}


private:
	void UpdatePosition();


public:
	void Initialize(int bubble_num, int bubble_gap, Vector2 bubble_size, Vector2 anchor);
	void SetTexture(int bubble_index, std::wstring_view path);

	void SmoothAppear(int bubble_index);
	void SmoothDisappear(int bubble_index);
	void InstantAppear(int bubble_index);
	void InstantDisappear(int bubble_indx);
};

