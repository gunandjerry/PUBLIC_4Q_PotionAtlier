#include "SceneManager.h"
#include <GameObject\Base\CameraObject.h>
#include <Core/DXTKInputSystem.h>
#include <D3DCore/D3D11_GameApp.h>
#include <Component/Render/MeshRender.h>
#include <filesystem>
#include <ranges>
#include <algorithm>
#include <Manager/InstanceIDManager.h>
#include <Core/TimeSystem.h>
#include <Component/Render/SkyBoxRender.h>
#include <regex>

SceneManager& sceneManager = SceneManager::GetInstance();

void SceneManager::LoadScene(const wchar_t* scenePath)
{
	WaitThreads();

	std::unordered_set<unsigned int> dontDestroyIDs;
	dontDestroyIDs.reserve(currScene->dontdestroyonloadList.size());
	for (auto& i : currScene->dontdestroyonloadList)
	{
		if (!i.expired())
		{
			dontDestroyIDs.insert(i.lock()->GetInstanceID());
		}
	}

	auto destroyList = currScene->objectList
		| std::views::filter([](auto& sptr){return sptr && !sptr->transform.Parent;})
		| std::views::filter(
		[&dontDestroyIDs](auto& sptr)
		{
			bool destroy = dontDestroyIDs.find(sptr->GetInstanceID()) == dontDestroyIDs.end();
			return destroy;
		});

	for (auto& obj : destroyList)
	{
		obj->componentList.clear();
		obj->renderList.clear();
		obj->startList.clear();
		DestroyObject(obj.get()); //파괴할 오브젝트 전부 제거
	}

	//루프 순서에 따른 SkyBoxRender 파괴 제어용 코드
	if (SkyBoxRender* skyBox = SkyBoxRender::mainSkyBox)
	{
		if (dontDestroyIDs.find(skyBox->gameObject.GetInstanceID()) == dontDestroyIDs.end()) //dontDestroyID 아닐때만 실행
			SkyBoxRender::mainSkyBox = nullptr; 
	}

	currScene->loadScenesMap.clear();
	currScene->sceneName = std::filesystem::path(scenePath).filename();

	PhysicsManager::ClearPhysicsScene();

	gameObjectFactory.DeserializedScene(currScene.get(), scenePath);
	gameObjectFactory.CompactObjectMemoryPool();

	lastLoadScenePath = scenePath;

	TimeSystem::Time.ClearInvokFunc();
	TimeSystem::Time.UpdateTime();
}

void SceneManager::AddScene(const wchar_t* scenePath)
{
	if (currScene)
	{
		std::wstring sceneName = std::filesystem::path(scenePath).filename();
		auto find = currScene->loadScenesMap.find(sceneName);
		if (find != currScene->loadScenesMap.end())
		{
			MessageBox(NULL, L"이미 로드된 Scene 입니다.", sceneName.c_str(), MB_OK);
		}
		currScene->sceneName = std::filesystem::path(scenePath).filename();
		gameObjectFactory.DeserializedScene(currScene.get(), scenePath);
	}
}

void SceneManager::SubScene(const wchar_t* scenefileName)
{
	if (currScene)
	{
		auto find = currScene->loadScenesMap.find(scenefileName);
		if (find != currScene->loadScenesMap.end())
		{
			for (auto& weakptr : find->second)
			{
				if (!weakptr.expired())
				{
					DestroyObject(weakptr.lock().get());
				}
			}
			currScene->loadScenesMap.erase(find);
		}
	}
}

void SceneManager::SaveScene(const wchar_t* savePath, bool Override)
{
	gameObjectFactory.SerializedScene(currScene.get(), savePath, Override);
}

void SceneManager::AddGameObject(std::shared_ptr<GameObject>& object)
{
	if (sceneManager.nextScene)
		sceneManager.nextAddQueue.push(object);
	else
		sceneManager.currAddQueue.push(object);

	if (!isStartScene)
		AddObjects();
}

SceneManager::SceneManager()
{
	
}

SceneManager::~SceneManager()
{

}

void SceneManager::DestroyObject(GameObject* obj)
{
	eraseSet.insert(obj);
	EraseObjectFindMap(obj);
	if (obj->transform.GetChildCount() > 0)
	{
		for (Transform* childObj : obj->transform.childList)
		{
			DestroyObject(childObj->_gameObject);
		}
	}
}

void SceneManager::DestroyObject(GameObject& obj)
{
	DestroyObject(&obj);
}

void SceneManager::DontDestroyOnLoad(GameObject* obj)
{
	obj = obj->transform.rootParent ? &obj->transform.rootParent->gameObject : obj;
	std::vector<std::weak_ptr<GameObject>>* dontDestroyList = nullptr;
	if (nextScene)
	{
		dontDestroyList = &nextScene->dontdestroyonloadList;
	}
	else
	{
		dontDestroyList = &currScene->dontdestroyonloadList;
	}
	dontDestroyList->emplace_back(obj->GetWeakPtr());

	if (obj->transform.GetChildCount() > 0)
	{
		std::vector<Transform*> transformStack;
		for (Transform* childObj : obj->transform.childList)
		{
			transformStack.push_back(childObj);
		}
		while (!transformStack.empty())
		{
			Transform* currObject = transformStack.back();
			transformStack.pop_back();
			dontDestroyList->emplace_back(currObject->_gameObject->GetWeakPtr());
			for (Transform* childObj : currObject->childList)
			{
				transformStack.push_back(childObj);
			}
		}
	}
	std::erase_if(*dontDestroyList, [](std::weak_ptr<GameObject> weakPtr)
		{
			return weakPtr.expired();
		});
}

void SceneManager::DontDestroyOnLoad(GameObject& obj)
{
	DontDestroyOnLoad(&obj);
}

size_t SceneManager::GetObjectsCount()
{
	if (currScene)
	{
		size_t count = 0;
		for (auto& item : currScene->objectList)
		{
			if (item.get())
				++count;
		}
		return count;
	}
	else return 0;
}

ObjectList SceneManager::GetObjectList()
{
	if (currScene)
	{
		std::vector<GameObject*> objList;
		objList.reserve((GetObjectsCount()));
		for (int i = 0; i < currScene->objectList.size(); i++)
		{
			if (GameObject* obj = currScene->objectList[i].get())
			{
				objList.push_back(obj);
			}
		}
		return objList;
	}
	else return std::vector<GameObject*>();
}

std::vector<std::wstring> SceneManager::GetSceneList()
{
	std::vector<std::wstring> sceneList;
	if(currScene)
	{
		sceneList.reserve(currScene->loadScenesMap.size());
		for (auto& [key, objList] : currScene->loadScenesMap)
		{
			sceneList.push_back(key);
		}
	}
	return sceneList;
}

bool SceneManager::IsImGuiActive()
{
	if (currScene)
		return currScene->UseImGUI;
	else
		return false;
}

void SceneManager::PushImGuiPopupFunc(const std::function<void()>& func)
{
	if (nextScene)
		nextScene->ImGUIPopupQue.push(func);
	else if (currScene)
		currScene->ImGUIPopupQue.push(func);
}

void SceneManager::PopImGuiPopupFunc()
{
	if (currScene)
		currScene->ImGUIPopupQue.pop();
}

void SceneManager::SetLodingImguiFunc(const std::function<void()>& func)
{
	if (ImGuiLodingFunc)
	{
		__debugbreak(); //로딩 화면이 겹침.
	}		
	ImGuiLodingFunc = func;
}

void SceneManager::EndLodingImguiFunc()
{
	ImGuiLodingFunc = nullptr;
}

void SceneManager::PushAsynCallBackFunc(const std::function<void()>& callBack)
{
	asynCallBackMutex.lock();
	asynCallBackVec.push_back(callBack);
	asynCallBackMutex.unlock();
}

GameObject* SceneManager::FindObject(const wchar_t* name)
{

	GameObject* obj = nullptr;
	auto findIter = objectFindMap2.find(name);
	if (findIter != objectFindMap2.end())
	{
		obj = findIter->second;
	}
	return obj;
}

GameObject* SceneManager::GetObjectToID(unsigned int instanceID)
{
	GameObject* obj = nullptr;
	if (currScene && instanceID < currScene->objectList.size())
	{
		obj = currScene->objectList[instanceID].get();
	}
	return obj;
}

void SceneManager::FixedUpdateScene()
{
	currScene->FixedUpdate();
	currScene->PhysicsUpdate(TimeSystem::Time.GetFixedDelta());
}

void SceneManager::UpdateScene()
{
	if (!asynCallBackVec.empty())
	{
		asynCallBackMutex.lock();
		for (auto& func : asynCallBackVec)
		{
			func();
		}
		asynCallBackVec.clear();
		asynCallBackMutex.unlock();
	}
	currScene->Update();
}

void SceneManager::LateUpdateScene()
{
	currScene->LateUpdate();
}

void SceneManager::RenderScene()
{
	if (ImGuiLodingFunc)
	{
		Scene::ImGUIBegineDraw();
		ImGuiLodingFunc();
		Scene::ImGUIEndDraw();
		D3D11_GameApp::Present();
	}
	else
	{
		currScene->Render();
	}
}

void SceneManager::AddObjects()
{
	if (ImGuiLodingFunc)
		return;

	while (!currAddQueue.empty())
	{
		auto& obj = currAddQueue.front();
		AddObjectCurrScene(obj);
		obj->transform.lockUpdate = false;
		obj->transform.UpdateTransform();
		obj->transform.ResetFlagUpdateWM();	
		currAddQueue.pop();
	}
}

void SceneManager::EraseObjects()
{
	if (ImGuiLodingFunc)
		return;

	if (!eraseSet.empty())
	{
		for (auto& obj : eraseSet)
		{
			EraseObject(obj->GetInstanceID());
		}
		eraseSet.clear();

		auto& objList = currScene->objectList;
		while (!objList.empty() && objList.back() == nullptr)
		{
			objList.pop_back();
		}
	}
	if (!eraseComponentSet.empty())
	{
		for (auto& component : eraseComponentSet)
		{
			component->gameObject.EraseComponent(component);
		}
		eraseComponentSet.clear();
	}
	instanceIDManager.SortReturnID();
}

void SceneManager::ChangeScene()
{
	if (ImGuiLodingFunc)
		return;

	if (nextScene && !ImGuiLodingFunc)
	{
		WaitThreads();

		while (!nextAddQueue.empty())
		{
			auto& obj = nextAddQueue.front();
			AddObjectNextScene(obj);
			nextAddQueue.pop();
		}
		if (currScene)
		{
			std::erase_if(currScene->dontdestroyonloadList, [](std::weak_ptr<GameObject> ptr) {return ptr.expired(); });
			if (!currScene->dontdestroyonloadList.empty())
			{
				nextScene->dontdestroyonloadList = std::move(currScene->dontdestroyonloadList);
				for (auto& weakptr : nextScene->dontdestroyonloadList)
				{
					if (!weakptr.expired())
					{
						unsigned int id = weakptr.lock()->GetInstanceID();
						if (nextScene->objectList.size() <= id)
							nextScene->objectList.resize((size_t)id + 1);

						nextScene->objectList[id] = currScene->objectList[id];
					}
				}
			}
			currScene.reset();
			gameObjectFactory.CompactObjectMemoryPool();
			MeshRender::ReloadShaderAll(); //유효한 메시 객체들 셰이더 다시 생성
		}
		currScene = std::move(nextScene);
		isStartScene = false;
		currScene->Awake();	
		isStartScene = true;
	}
	else if (EndGame)
	{
		WaitThreads();
		WinGameApp::GameEnd();
		currScene.reset();
		nextScene.reset();
	}
}

void SceneManager::AddObjectCurrScene(std::shared_ptr<GameObject>& obj)
{
	if (!currScene) return;
	unsigned int id = obj->GetInstanceID();
	objectFindMap[obj->Name] = obj->GetInstanceID();
	objectFindMap2[obj->Name] = obj.get();
	if (id >= currScene->objectList.size())
	{
		currScene->objectList.resize((size_t)id + 1);
	}
	currScene->objectList[id] = obj;
}

void SceneManager::AddObjectNextScene(std::shared_ptr<GameObject>& obj)
{
	unsigned int id = obj->GetInstanceID();
	objectFindMap[obj->Name] = obj->GetInstanceID();
	objectFindMap2[obj->Name] = obj.get();
	if (id >= nextScene->objectList.size())
	{
		nextScene->objectList.resize((size_t)id + 1);
	}
	nextScene->objectList[id] = obj;
}

std::wstring SceneManager::ChangeObjectName(unsigned int instanceID, const std::wstring& _pervName, const std::wstring& _newName)
{
	const wchar_t* newNameCStr = _newName.c_str();

	//이름 중복 방지
	std::wstring newNameTemp;
	auto newNameIter = objectFindMap.find(_newName);
	if (newNameIter != objectFindMap.end())
	{
		newNameTemp = _newName;
	}
	while (newNameIter != objectFindMap.end())
	{
		auto incrementObjectName = [](const std::wstring& name)->std::wstring
			{
				std::wregex pattern(LR"(^(.*?)(?:\s*\((\d+)\))?$)");
				std::wsmatch match;
				if (std::regex_match(name, match, pattern)) {
					std::wstring baseName = match[1].str();
					int number = match[2].matched ? std::stoi(match[2].str()) : -1;
					return baseName + L" (" + std::to_wstring(number + 1) + L")";
				}
				return name;
			};

		newNameTemp = incrementObjectName(newNameTemp);
		newNameIter = objectFindMap.find(newNameTemp);
		newNameCStr = newNameTemp.c_str();
	}
	objectFindMap[newNameCStr] = instanceID;
	auto pervNameIter = objectFindMap.find(_pervName);
	if (pervNameIter != objectFindMap.end())
	{
		objectFindMap.erase(pervNameIter);
	}
	return newNameCStr;
}

void SceneManager::EraseObject(unsigned int id)
{
	if(id < currScene->objectList.size())
		currScene->objectList[id].reset(); 
}

void SceneManager::EraseObjectFindMap(GameObject* obj)
{
	auto findIter = objectFindMap.find(obj->Name);
	if (findIter == objectFindMap.end())
		return;

	objectFindMap.erase(findIter);
	objectFindMap2.erase(obj->Name);
}

void SceneManager::WaitThreads()
{
	gameObjectFactory.DeserializedThreadsAsync();
	if(!asynCallBackVec.empty())
		asynCallBackVec.clear();
}


