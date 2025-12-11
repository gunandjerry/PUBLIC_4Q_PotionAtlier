#include "GameStartButton.h"
#include "Object/GameManager.h"
#include "Components/GameManagerComponent.h"
#include "Components/BoingBoing.h"
#include "Object\AudioPlayerObject.h"
#include "Components/CustomerSpawner.h"

class GameStartHelperComponent : public Component
{
public:
	virtual void Awake() {}
	virtual void Start() {}
	std::string nextScenePath;
	
	virtual void Serialized(std::ofstream& ofs) 
	{
		Binary::Write::string(ofs, nextScenePath);
	};

	virtual void Deserialized(std::ifstream& ifs) 
	{
		nextScenePath = Binary::Read::string(ifs);
	};
	virtual void InspectorImguiDraw() 
	{
		ImGui::InputText("Next Scene Path", (char*)nextScenePath.c_str(), nextScenePath.size(), ImGuiInputTextFlags_CallbackResize,
						 [](ImGuiInputTextCallbackData* data)
						 {
							 if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
							 {
								 std::string* str = (std::string*)data->UserData;
								 IM_ASSERT(data->Buf == str->c_str());
								 str->resize(data->BufTextLen);
								 data->Buf = str->data();
							 }
							 return 0;
						 }, &nextScenePath);
	}
protected:
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}


};

class GameStartHelperObejct : public GameObject
{
public:
	SERIALIZED_OBJECT(GameStartHelperObejct)
	GameStartHelperObejct()
	{ 
		component = &AddComponent<GameStartHelperComponent>();
	}
	GameStartHelperComponent* component;
};



class GameStartButtonComponent : public Component
{
public:
	virtual void Awake() {}
	virtual void Start() 
	{
		helper = GameObject::Find<GameStartHelperObejct>(L"GameStartHelperObejct");
		timer = 0;
	}
	GameStartHelperObejct* helper = nullptr;
	bool isEvent = false;
protected:
	float timer = 0;
	virtual void FixedUpdate() {}
	virtual void Update() 
	{
		auto& input = inputManager.input;
		if (input.IsKeyDown(KeyboardKeys::Space))
		{
			GameStartButton* ButtonObject = dynamic_cast<GameStartButton*>(&gameObject);
			if (ButtonObject)
			{
				ButtonObject->eventListener.InvokeOnClickUp();
			}
		}

		if (!isEvent) return;
		timer += TimeSystem::Time.DeltaTime;
		if (timer > 1.2f)
		{
			std::wstring wstr = L"EngineResource/CutSceneScene.Scene";
			if (helper)
			{
				wstr.assign(helper->component->nextScenePath.begin(), helper->component->nextScenePath.end());
			}
			sceneManager.LoadScene(wstr.c_str());
		}
	}
	virtual void LateUpdate() {}

};

GameStartButton::GameStartButton() : 
	ButtonObjectBase(),
	boing(AddComponent<BoingBoingUI>())
{
	AddComponent< GameStartButtonComponent>();
}


void GameStartButton::Awake()
{
	boing.use_on_ui_render2 = true;
	eventListener.SetOnClickUp(
		[this]()
		{
			eventListener.SetOnClickUp([]() {});
			if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Click"))
			{
				findItem->IsComponent<AudioBankClip>()->Play();
			}

			boing.Boing(0);
			auto event = GameObject::Find<SceneEventObject>(L"SceneEventObject");
			if (event)
			{
				event->componenet->EventStart();
			}

			IsComponent< GameStartButtonComponent>()->isEvent = true;
		});
}


SceneEventObject::SceneEventObject() : GameObject()
{
	componenet = &AddComponent<SceneEventComponent>();
}

void SceneEventComponent::EventStart()
{
	isEvent = true;
	
	currentTime = 0;

	UI->gameObject.Active = true;
	UI2->gameObject.Active = true;
	UI->anchorMin.x = 0.5f;
	UI->anchorMin.y = 0.5f;
	UI->anchorMax.x = 0.5f;
	UI->anchorMax.y = 0.5f;
}

void SceneEventComponent::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, 1);
	Binary::Write::data(ofs, timer);
	Binary::Write::data(ofs, isReverse);
	Binary::Write::data(ofs, isEventStart);

}

void SceneEventComponent::Deserialized(std::ifstream& ifs)
{
	int version = Binary::Read::data<int>(ifs);
	timer = Binary::Read::data<float>(ifs);
	isReverse = Binary::Read::data<bool>(ifs);
	isEventStart = Binary::Read::data<bool>(ifs);
}

void SceneEventComponent::InspectorImguiDraw()
{
	ImGui::Checkbox("isReverse", &isReverse);
	ImGui::Checkbox("isEventStart", &isEventStart);
	ImGui::DragFloat("timer", &timer, 0.01f, 0, 100);
}


void SceneEventComponent::Update()
{
	if (!isEvent) 		return;

	if (GameManagerComponent* gm = GameManager::IsGM())
	{
		//if (!CustomerSpawner::IsInit()) return;

		if (!gm->IsInit())
		{
			return;
		}
		

	}
	if (TimeSystem::Time.DeltaTime > timer) 
		return;

	currentTime += TimeSystem::Time.DeltaTime;
	if (currentTime > timer)
	{
		if (UI)
		{
			UI->gameObject.Active = false;
		}
		if (isReverse && UI2)
		{
			UI2->gameObject.Active = false;
		}
	}
	if (UI)
	{
		float timepercent = currentTime / timer;
		if (isReverse)
		{
			UI->anchorMin.x = 0.5f - timepercent * 0.5f;
			UI->anchorMin.y = 0.5f - timepercent * 0.5f;
			UI->anchorMax.x = 0.5f + timepercent * 0.5f;
			UI->anchorMax.y = 0.5f + timepercent * 0.5f;
		}
		else
		{
			UI->anchorMin.x = timepercent * 0.5f;
			UI->anchorMin.y = timepercent * 0.5f;
			UI->anchorMax.x = 1 - timepercent * 0.5f;
			UI->anchorMax.y = 1 - timepercent * 0.5f;
		}
		UI->SetTransform();
	}
}

void SceneEventComponent::Start()
{
	auto gameStartUI = GameObject::Find(L"GameStartUI");
	gameStartUI->Active = false;
	UI = gameStartUI->IsComponent<UIRenderComponenet2>();

	auto gameStartUI2 = GameObject::Find(L"GameStartUI2");
	gameStartUI2->Active = false;
	UI2 = gameStartUI2->IsComponent<UIRenderComponenet2>();

	if (isEventStart)
	{
		EventStart();
	}
}

