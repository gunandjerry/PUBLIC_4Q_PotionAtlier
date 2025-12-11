#pragma once
#include "framework.h"
#include <Component/Base/Component.h>

struct BoingPath
{
	std::string boing_name;
	std::vector<Vector3> paths;
};

class BoingBoing : public Component
{
	std::vector<BoingPath> boings;

	float boing_speed{ 10.0f };
	float t{ 0.0f };
	int boing_idx{ 0 };

	BoingPath* current_boing{ nullptr };
	Vector3 original_scale{ 1, 1, 1 };
	class AudioBankClip* SFX_Boing{ nullptr };

	//std::vector<std::pair<GameObject*, Vector3>> targets;

public:
	virtual void Awake();

	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}

	virtual void InspectorImguiDraw() override;


public:
	void Boing(int idx);
//private:
//	void FindMeshesRecursively(Transform* parent);
};




struct BoingUIPath
{
	std::string boing_name;
	std::vector<Vector2> paths;
};

class BoingBoingUI : public Component
{
	std::vector<BoingUIPath> boings;

	float boing_speed{ 10.0f };
	float t{ 0.0f };
	int boing_idx{ 0 };

	BoingUIPath* current_boing{ nullptr };
	Vector2 original_scale{ 1, 1 };

	class UIRenderComponenet* ui_render1{ nullptr };
	class UIRenderComponenet2* ui_render2{ nullptr };
public:
	bool use_on_ui_render2{ false };

	virtual void Awake();

	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}

	virtual void InspectorImguiDraw() override;


public:
	void Boing(int idx);
	//private:
	//	void FindMeshesRecursively(Transform* parent);
};

