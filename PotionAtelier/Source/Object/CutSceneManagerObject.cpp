#include "CutSceneManagerObject.h"
#include "Object/GameManager.h"
#include "Object/ButtonObject/GameStartButton.h"	
#include "Components/GameManagerComponent.h"

class CutSceneComponet : public Component
{
public:
	virtual void Awake() {}
	virtual void Start()
	{
		if (!isWakeup)
		{
			UI = IsComponent<UIRenderComponenet2>();
			UI->Enable = false;

		}

	}
	void WakeUp()
	{
		UI = IsComponent<UIRenderComponenet2>();
		isWakeup = true;
		UI->Enable = true;
	}
	bool IsWakeupEnd() { return isWakeupEnd; }
protected:
	virtual void FixedUpdate() {}
	virtual void Update()
	{
		if (!isWakeup) return;
		currTime += TimeSystem::Time.DeltaTime;
		float percent = currTime / time;
		percent = std::clamp(percent, 0.0f, 1.0f);

		Vector3 pos = Vector3::Lerp(start, end, percent);
		UI->SetPosX(pos.x);
		UI->SetPosY(pos.y);
		UI->GetCustomData().SetField("CutScenePercent", percent);
		if (currTime > time)
		{
			isWakeupEnd = true;
		}
	}
	virtual void LateUpdate() {}


	Vector3 start;
	Vector3 end;
	float time{1.0f};
	float currTime;
	bool isWakeup;
	bool isWakeupEnd;
	UIRenderComponenet2* UI;


	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Serialized(std::ofstream& ofs) 
	{
		Binary::Write::data(ofs, 1);
		Binary::Write::Vector3(ofs, start);
		Binary::Write::Vector3(ofs, end);
		Binary::Write::data(ofs, time);
	};
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Deserialized(std::ifstream& ifs) 
	{
		int version = Binary::Read::data<int>(ifs);
		start = Binary::Read::Vector3(ifs);
		end = Binary::Read::Vector3(ifs);
		time = Binary::Read::data<float>(ifs);
	};
	virtual void InspectorImguiDraw() 
	{
		if (ImGui::TreeNode("CutSceneComponet"))
		{
			ImGui::DragFloat3("start", &start.x, 0.1f);
			ImGui::DragFloat3("end", &end.x, 0.1f);
			ImGui::DragFloat("time", &time, 0.1f);
			ImGui::TreePop();
		}

	}
};


class CutSceneManagerComponet : public Component
{
public:
	virtual void Awake() {}
	virtual void Start() 
	{
		step = 0;
		UI = dynamic_cast<CutSceneObject*> (GameObject::Find(std::format(L"CutSceneObject{}", step).c_str()));
		if (!UI) GameSceneLoad();
		UI->componenet->WakeUp();
	}
	void GameSceneLoad()
	{
		isSceneLoad = true;
		auto event = GameObject::Find<SceneEventObject>(L"SceneEventObject");
		if (event)
		{
			event->componenet->isReverse = false;
			event->componenet->EventStart();
			auto ui = event->componenet->UI;
			if (ui)
			{
				ui->SetPosX(0);
				ui->SetPosY(0);
			}
		}
	}
protected:
	virtual void FixedUpdate() {}
	virtual void Update() 
	{

#ifdef _EDITOR
		if (!Scene::EditorSetting.IsPlay()) return;
#endif

		auto& input = inputManager.input;
		if (input.IsKeyDown(KeyboardKeys::Escape))
		{
			GameSceneLoad();
		}

		if (isSceneLoad)
		{
			waitTIme += TimeSystem::Time.DeltaTime;
			if (waitTIme >= 1.2f)
			{
				isSceneLoad = false;
				GameManager::GetGM().StageLoad(0);
			}
		}
		else
		{
			if (UI && !UI->componenet->IsWakeupEnd()) return;
			if (input.GetMouseState().leftButton || input.IsKeyDown(KeyboardKeys::Space))
			{
				++step;
				UI = dynamic_cast<CutSceneObject*> (GameObject::Find(std::format(L"CutSceneObject{}", step).c_str()));
				if (!UI)
				{
					GameSceneLoad();
				}
				else
				{
					UI->componenet->WakeUp();
				}
			}
		}

	}
	virtual void LateUpdate() {}
	
	float waitTIme = 0.0f;
	bool isSceneLoad = false;
private:
	CutSceneObject* UI;



	int step;
};




CutSceneObject::CutSceneObject()
{
	componenet = &AddComponent<CutSceneComponet>();
	AddComponent<UIRenderComponenet2>();
}

CutSceneObject::~CutSceneObject()
{
}

CutSceneManagerObject::CutSceneManagerObject()
{
	AddComponent<CutSceneManagerComponet>();
}

CutSceneManagerObject::~CutSceneManagerObject()
{
}

void CutSceneManagerObject::Skip()
{
	CutSceneManagerComponet* ee = IsComponent<CutSceneManagerComponet>();
	if (ee)
	{
		ee->GameSceneLoad();
	}
}




class CutSceneManagerComponet2 : public Component
{
public:
	virtual void Awake() {}
	virtual void Start()
	{
		step = 0;
		UI = dynamic_cast<CutSceneObject*> (GameObject::Find(std::format(L"CutSceneObject{}", step).c_str()));
		if (!UI) GameSceneLoad();
		UI->componenet->WakeUp();
	}
protected:
	virtual void FixedUpdate() {}
	virtual void Update()
	{

#ifdef _EDITOR
		if (!Scene::EditorSetting.IsPlay()) return;
#endif
		if (isSceneLoad)
		{
			waitTIme += TimeSystem::Time.DeltaTime;
			if (waitTIme >= 1.2f)
			{
				isSceneLoad = false;

				if (nextScenenPath.size())
				{
					std::wstring wstr = std::wstring(nextScenenPath.begin(), nextScenenPath.end());
					sceneManager.LoadScene(wstr.c_str());
				}
			}
		}
		else
		{
			if (UI && !UI->componenet->IsWakeupEnd()) return;
			auto& input = inputManager.input;
			if (input.GetMouseState().leftButton)
			{
				++step;
				UI = dynamic_cast<CutSceneObject*> (GameObject::Find(std::format(L"CutSceneObject{}", step).c_str()));
				if (!UI)
				{
					GameSceneLoad();
				}
				else
				{
					UI->componenet->WakeUp();
				}
			}
		}

	}
	virtual void LateUpdate() {}
	void GameSceneLoad()
	{
		isSceneLoad = true;
		auto event = GameObject::Find<SceneEventObject>(L"SceneEventObject");
		if (event)
		{
			event->componenet->isReverse = false;
			event->componenet->EventStart();
			auto ui = event->componenet->UI;
			if (ui)
			{
				ui->SetPosX(0);
				ui->SetPosY(0);
			}
		}
	}
	virtual void InspectorImguiDraw()
	{
		ImGui::InputText("Next Scene Path", (char*)nextScenenPath.c_str(), nextScenenPath.size(), ImGuiInputTextFlags_CallbackResize,
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
						 }, & nextScenenPath);



	}

	virtual void Serialized(std::ofstream& ofs)
	{
		Binary::Write::string(ofs, nextScenenPath);
	}

	virtual void Deserialized(std::ifstream& ifs)
	{
		nextScenenPath = Binary::Read::string(ifs);
	}
	std::string nextScenenPath;
	float waitTIme = 0.0f;
	bool isSceneLoad = false;
private:
	CutSceneObject* UI;
	int step;
};


CutSceneManagerObject2::CutSceneManagerObject2()
{
	AddComponent<CutSceneManagerComponet2>();
}

CutSceneManagerObject2::~CutSceneManagerObject2()
{
}
