#pragma once
#include <functional>
#include "Component/Base/Component.h"

class EventListener : public Component
{
	friend class EventManager;

	std::function<void()> on_click_down{ nullptr };
	std::function<void()> on_click_up{ nullptr };
	std::function<void()> on_right_click_down{ nullptr };
	std::function<void()> on_right_click_up{ nullptr };


public:
	virtual void Awake() override {}
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}


public:
	void SetOnClickDown(std::function<void()> func) { this->on_click_down = func; }
	void SetOnClickUp(std::function<void()> func) { this->on_click_up = func; }
	void SetOnRightClickDown(std::function<void()> func) { this->on_right_click_down = func; }
	void SetOnRightClickUp(std::function<void()> func) { this->on_right_click_up = func; }

public:
	void InvokeOnClickDown() { if (on_click_down != nullptr) on_click_down(); }
	void InvokeOnClickUp() { if (on_click_up != nullptr) on_click_up(); }
	void InvokeOnRightClickDown() { if (on_right_click_down != nullptr) on_right_click_down(); }
	void InvokeOnRightClickUp() { if (on_right_click_up != nullptr) on_right_click_up(); }
};

