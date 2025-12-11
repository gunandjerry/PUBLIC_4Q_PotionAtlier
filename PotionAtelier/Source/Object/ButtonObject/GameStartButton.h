#pragma once
#include "ButtonObjectBase.h"

class SceneEventComponent : public Component
{
public:
	float timer = 0;
	float currentTime = 0;
	bool isReverse = false;
	bool isEventStart = false;
	float StartDelay = 0.0f;
	UIRenderComponenet2* UI;
	UIRenderComponenet2* UI2;

	void EventStart();

	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);
	virtual void InspectorImguiDraw();
	virtual void Awake() {}
protected:
	bool isEvent = false;
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}
	virtual void Start();
};

class SceneEventObject : public GameObject
{
	SERIALIZED_OBJECT(SceneEventObject)
public:
	SceneEventObject();
	virtual ~SceneEventObject() override = default;
	SceneEventComponent* componenet;
};

class BoingBoingUI;
class GameStartButton : public ButtonObjectBase
{
	SERIALIZED_OBJECT(GameStartButton)
public:
	GameStartButton();
	virtual ~GameStartButton() override = default;

	virtual void Awake() override;
	BoingBoingUI& boing;

};

