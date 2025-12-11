#pragma once
#include "framework.h"

class TutorialButtonComponent : public Component
{
	EventListener* event;
	std::vector<UIRenderComponenet*> uis;
public:
	virtual void Serialized(std::ofstream& ofs) {};
	virtual void Deserialized(std::ifstream& ifs) {};
	virtual void InspectorImguiDraw() {}
public:
	virtual void Awake() {}
	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}


private:
	bool show{ false };
	int show_idx{ 0 };
public:
	void Show();
private:
	void ShowPage(int idx);
	void HidePage(int idx);
};

