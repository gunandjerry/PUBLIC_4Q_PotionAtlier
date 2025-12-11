#include "GameObjectFactory.h"
#include <GameObject/Base/GameObject.h>
#include <framework.h>
#include <Utility/SerializedUtility.h>
#include <Component/Render/SimpleBoneMeshRender.h>
#include <ranges>
#include <algorithm>
#include <condition_variable>

GameObjectFactory& gameObjectFactory = GameObjectFactory::GetInstance();

void GameObjectFactory::RegisterParentFolder(const char* key, const char* filePath)
{
	auto& map = const_cast<std::map<std::string, std::string>&>(GetGameObjectParentFolderMap());
	std::filesystem::path path = filePath;
	map[key] = path.parent_path().filename().string();
}

GameObjectFactory::~GameObjectFactory()
{
	gameObjectMemoryPool.Uninitialize();
}

const std::map<std::string, std::string>& GameObjectFactory::GetGameObjectParentFolderMap()
{
	static std::map<std::string, std::string> gameObjectParentFolderMap;
	return gameObjectParentFolderMap;
}

void GameObjectFactory::GameObjectDeleter(GameObject* pObj)
{
	unsigned int id = pObj->GetInstanceID();
	pObj->~GameObject();
	gameObjectFactory.gameObjectMemoryPool.Free(id);
}

void GameObjectFactory::InitializeMemoryPool()
{
	gameObjectMemoryPool.Initialize(MaxGameObjectClassSize, MaxGameObjectClassSize * 500);
}

void GameObjectFactory::UninitializeMemoryPool()
{
	gameObjectMemoryPool.Uninitialize();
}

void GameObjectFactory::CompactObjectMemoryPool()
{
	gameObjectMemoryPool.CompactMemoryPage();
}

void* GameObjectFactory::GameObjectAlloc(size_t id)
{
	static std::mutex allocateMutex;
	std::lock_guard lock(allocateMutex);
	return gameObjectMemoryPool.Allocate(id);
}

std::function<GameObject*(const wchar_t* name)>& GameObjectFactory::NewGameObjectToKey(const char* key)
{
	NewObjectMapType& newGameObjectFuncMap = const_cast<NewObjectMapType&>(GetNewGameObjectFuncMap());
	auto findIter = newGameObjectFuncMap.find(key);
	if (findIter != newGameObjectFuncMap.end())
	{
		return findIter->second;
	}
	__debugbreak(); //존재하지 않는 키.
	throw std::runtime_error("Key not found in the map");
}

std::function<std::shared_ptr<GameObject>(const wchar_t* name)> GameObjectFactory::MakeGameObjectTokey(const char* key)
{
	ConstructorMapType& MakeGameObjectFuncMap = const_cast<ConstructorMapType&>(GetGameObjectConstructorMap());
	auto findIter = MakeGameObjectFuncMap.find(key);
	if (findIter != MakeGameObjectFuncMap.end())
	{
		return findIter->second;
	}
	__debugbreak(); //존재하지 않는 키.
	throw std::runtime_error("Key not found in the map");
}

const char* GameObjectFactory::GetObjectParentFolder(const char* key)
{
	auto& map = GetGameObjectParentFolderMap();
	auto find = map.find(key);
	if (find != map.end())
	{
		return find->second.c_str();
	}
	return nullptr;
}

void GameObjectFactory::SerializedScene(Scene* scene, const wchar_t* WritePath, bool isOverride)
{
	gameObjectFactory.DeserializedThreadsAsync();
	if (scene == nullptr)
	{
		__debugbreak(); //scene is a nullptr
		return;
	}

	std::filesystem::path path(WritePath);
	std::filesystem::path extension = path.extension();
	if (extension != L".Scene")
	{
		if (extension != L"")
		{
			path.replace_extension(L".Scene");
		}
		else
		{
			if (!Utility::IsPathDirectory(path))
			{
				path += L"/";
			}
			path += scene->GetSceneName();
			path += L".Scene";
		}
	}

	if (!std::filesystem::exists(path))
	{
		if (!std::filesystem::exists(path.parent_path()))
		{
			std::filesystem::create_directories(path.parent_path());
		}
	}
	else if (!isOverride)
	{
		int result = MessageBox(
			NULL,
			L"파일이 존재합니다. 덮어쓰시겠습니까?",
			path.c_str(),
			MB_OKCANCEL | MB_ICONQUESTION
		);
		if (result == IDCANCEL) {
			return;
		}
	}
	
	ObjectList objList = sceneManager.GetObjectList();
	auto parentObjects = objList | std::views::filter([](GameObject* obj) { return !obj->transform.Parent;});
	size_t objCount = std::ranges::distance(parentObjects);
	std::ofstream ofs(path.c_str(), std::ios::binary | std::ios::trunc);

	using namespace Binary;
	Write::data(ofs, std::numeric_limits< uint32_t>::max()); // header
	Write::data(ofs, 1); // version

	DefferdRenderer& mainRenderer = D3D11_GameApp::GetRenderer();
	Write::data(ofs, mainRenderer.directLight.size());
	for (uint32_t i = 0; i < mainRenderer.directLight.size(); i++)
	{
		auto& light = mainRenderer.directLight.GetDirectLight(i);
		Write::Vector4(ofs, light.Color);
		Write::Vector4(ofs, light.Directoin);
	}
	Write::data(ofs, mainRenderer.pointLight.size());
	for (uint32_t i = 0; i < mainRenderer.pointLight.size(); i++)
	{
		auto& light = mainRenderer.pointLight.GetPointLight(i);
		Write::Vector4(ofs, light.Color);
		Write::Vector4(ofs, light.position);
	}

	for (const auto& item : parentObjects)
	{
		Serialized(item, ofs, 0);
	}

	ofs.close();
}

void GameObjectFactory::DeserializedScene(Scene* scene, const wchar_t* ReadPath)
{
	std::filesystem::path path(ReadPath);
	if (!std::filesystem::exists(path))
	{
		MessageBox(NULL, L"파일이 존재하지 않습니다.", ReadPath, MB_OK);
		return;
	}
	if (path.extension() != L".Scene")
	{
		MessageBox(NULL, L"Scene 파일이 아닙니다.", ReadPath, MB_OK);
		return;
	}

	using namespace Binary;
	std::ifstream ifs(path.c_str(), std::ios::binary);
	if (!ifs.is_open())
	{
		return;
	}
	uint32_t header = Read::data<uint32_t>(ifs);
	int version = 0;
	uint32_t LightsCount;
	if (header == std::numeric_limits< uint32_t>::max())
	{
		version = Read::data<int>(ifs);
		LightsCount = Read::data<uint32_t>(ifs);
	}
	else
	{
		LightsCount = header;
	}

	DefferdRenderer& mainRenderer = D3D11_GameApp::GetRenderer();
	for (int i = (int)mainRenderer.directLight.size() - 1; i > 0; --i)
	{
		mainRenderer.directLight.PopDirectLight(std::format("Light {}", i).c_str());
	}
	DirectionLightData& mainLight = mainRenderer.directLight.GetDirectLight(0);
	mainLight.Color = Read::Vector4(ifs);
	mainLight.Directoin = Read::Vector4(ifs);
	for (uint32_t i = 1; i < LightsCount; i++)
	{
		DirectionLightData light{};
		light.Color = Read::Vector4(ifs);
		light.Directoin = Read::Vector4(ifs);
		mainRenderer.directLight.PushDirectLight(std::format("Light {}", i).c_str(), light);
	}
	for (int i = (int)mainRenderer.pointLight.size() - 1; i >= 0; --i)
	{
		mainRenderer.pointLight.PopPointLight(std::format("Light {}", i).c_str());
	}

	if (version >= 1)
	{
		LightsCount = Read::data<uint32_t>(ifs);
		PointLightData light{};
		for (uint32_t i = 0; i < LightsCount; i++)
		{
			light.Color = Read::Vector4(ifs);
			light.position = Read::Vector4(ifs);
			mainRenderer.pointLight.PushPointLight(std::format("Light {}", i).c_str(), light);
		}

	}

	static std::vector<std::shared_ptr<GameObject>> newObjectList;
	newObjectList = Deserialized(ifs, scene);
	for (auto& item : newObjectList)
	{
		sceneManager.AddGameObject(item);
	}
	newObjectList.clear();
	ifs.close();
}

void GameObjectFactory::SerializedObject(GameObject* object, const wchar_t* WritePath, bool isOverride)
{
	if (object == nullptr)
	{
		__debugbreak(); //존재하지 않는 오브젝트
		return;
	}

	NewObjectMapType* newGameObjectFuncMap = const_cast<NewObjectMapType*>(&GetNewGameObjectFuncMap());
	if (newGameObjectFuncMap->find(typeid(*object).name()) == newGameObjectFuncMap->end())
	{
		MessageBox(NULL, L"Is not Serialized Object", object->Name.c_str(), MB_OK);
		return;
	}
	std::filesystem::path path(WritePath);
	std::filesystem::path extension = path.extension();
	if (extension != L".GameObject")
	{
		if (extension != L"")
		{
			path.replace_extension(L".GameObject");
		}
		else
		{
			if (!Utility::IsPathDirectory(path))
			{
				path += L"/";
			}
			path += object->GetName();
			path += L".GameObject";
		}
	}

	if (!std::filesystem::exists(path))
	{
		if (!std::filesystem::exists(path.parent_path()))
		{
			std::filesystem::create_directories(path.parent_path());
		}
	}
	else if(!isOverride)
	{
		int result = MessageBox(
			NULL,                      
			L"파일이 존재합니다. 덮어쓰시겠습니까?", 
			path.c_str(),
			MB_OKCANCEL | MB_ICONQUESTION 
		);
		if (result == IDCANCEL) {
			return;
		}
	}	

	std::ofstream ofs(path.c_str(), std::ios::binary | std::ios::trunc);
	Serialized(object, ofs, 0);
	ofs.close();
}

GameObject* GameObjectFactory::DeserializedObject(const wchar_t* ReadPath)
{
	std::filesystem::path path(ReadPath);
	if (!std::filesystem::exists(path))
	{
		MessageBox(NULL, L"파일이 존재하지 않습니다.", ReadPath, MB_OK);
		return nullptr;
	}
	if (path.extension() != L".GameObject")
	{
		MessageBox(NULL, L"GameObject 파일이 아닙니다.", ReadPath, MB_OK);
		return nullptr;
	}

	deserializedMutex.lock();
	std::ifstream ifs(path.c_str(), std::ios::binary);
	auto objList = Deserialized(ifs);
	ifs.close();
	for (auto& item : objList)
	{
		sceneManager.AddGameObject(item);
	}
	deserializedMutex.unlock();
	return objList.front().get();
}

static HANDLE asynEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
static std::vector<HANDLE> waitMainEvent;
void GameObjectFactory::DeserializedObjectAsync(const wchar_t* ReadPath, const std::function<void(GameObject* object)>& endCallBackFunc)
{
	std::filesystem::path path(ReadPath);
	if (!std::filesystem::exists(path))
	{
		MessageBox(NULL, L"파일이 존재하지 않습니다.", ReadPath, MB_OK);
		return;
	}
	if (path.extension() != L".GameObject")
	{
		MessageBox(NULL, L"GameObject 파일이 아닙니다.", ReadPath, MB_OK);
		return;
	}

	static std::vector<std::shared_ptr<GameObject>> objList;
	waitMainEvent.push_back(CreateEvent(nullptr, FALSE, FALSE, nullptr));
	int index = deserializedThreads.size();
	deserializedThreads.emplace_back(
		[path, this, endCallBackFunc, index]()
		{
			deserializedMutex.lock();
			std::ifstream ifs(path.c_str(), std::ios::binary);
			objList = Deserialized(ifs);
			ifs.close();	
			sceneManager.PushAsynCallBackFunc(
				[endCallBackFunc]
				{
					endCallBackFunc(objList.front().get());
					for (auto& item : objList)
					{
						sceneManager.AddGameObject(item);
					}
					objList.clear();
					SetEvent(asynEvent);
				});		
			SetEvent(waitMainEvent[index]);
			WaitForSingleObject(asynEvent, INFINITE);
			deserializedMutex.unlock();
		});
}

void GameObjectFactory::DeserializedThreadsAsync()
{
	if (waitMainEvent.empty())
		return;

	for (int i = 0; i < waitMainEvent.size(); i++)
	{
		WaitForSingleObject(waitMainEvent[i], INFINITE);
		SetEvent(asynEvent);
		deserializedThreads[i].join();
		CloseHandle(waitMainEvent[i]);
	}
	deserializedThreads.clear();
	waitMainEvent.clear();
}

const std::map<std::string, std::function<GameObject* (const wchar_t* name)>>& GameObjectFactory::GetNewGameObjectFuncMap()
{
	static std::map<std::string, std::function<GameObject* (const wchar_t* name)>> newGameObjectFuncMap;
	return newGameObjectFuncMap;	
}

const std::map<std::string, std::function<std::shared_ptr<GameObject> (const wchar_t* name)>>& GameObjectFactory::GetGameObjectConstructorMap()
{
	static std::map<std::string, std::function<std::shared_ptr<GameObject>(const wchar_t* name)>> gameObjectConstructorMap;
	return gameObjectConstructorMap;
}

void GameObjectFactory::Serialized(GameObject* object, std::ofstream& ofs, size_t level)
{
	constexpr size_t SERIALIZED_VERSION = 1;

	using namespace Binary;
	const char* typeName = typeid(*object).name();
	Write::string(ofs, typeName);
	const wchar_t* objName = object->Name.c_str();
	Write::wstring(ofs, objName);
	if constexpr (SERIALIZED_VERSION > 0)
	{
		Write::data(ofs, object->tagSet.size());
		for (auto& tag : object->tagSet)
		{
			Write::wstring(ofs, tag);
		}
	}
	Write::data(ofs, object->Active);
	Write::BoundingBox(ofs, object->Bounds);
	object->Serialized(ofs);

	//Components
	for (auto& component : object->componentList)
	{
		component->Serialized(ofs);
	}

	//Transform
	Write::data<size_t>(ofs, level);
	bool isParent = level > 0;
	if (!isParent)
	{
		Write::Vector3(ofs, object->transform.position);
		Write::Quaternion(ofs, object->transform.rotation);
		Write::Vector3(ofs, object->transform.scale);
	}
	else
	{
		Write::Vector3(ofs, object->transform.localPosition);
		Write::Quaternion(ofs, object->transform.localRotation);
		Write::Vector3(ofs, object->transform.localScale);
	}	

	//TransformAnimation
	TransformAnimation* animation = object->IsComponent<TransformAnimation>();
	bool isTransformAnimation = animation != nullptr;
	Write::data(ofs, isTransformAnimation);
	if (animation)
		animation->SerializedAnimation(ofs);

	//childs
	for (size_t i = 0; i < object->transform.GetChildCount(); i++)
	{
		if(GameObject* chidObject = &object->transform.GetChild(i)->gameObject)
			Serialized(chidObject, ofs, level + 1);
	}
}

std::vector<std::shared_ptr<GameObject>> GameObjectFactory::Deserialized(std::ifstream& ifs, Scene* scene)
{
	constexpr size_t DESERIALIZED_VERSION = 1;

	thread_local std::map<size_t, Transform*> objectTreeMap;
	std::vector<std::shared_ptr<GameObject>> makeObjectList;
	std::vector<TransformAnimation*> transformAnimationVec;
	GameObject* root = nullptr;
	using namespace Binary;
	while (true)
	{
		std::string typeName = Read::string(ifs);
		if (ifs.eof())
		{
			objectTreeMap.clear();
			for (auto& animation : transformAnimationVec)
			{
				animation->AddChildrenToTargets();
			}
			SimpleBoneMeshRender::EndDeserialized();
			return makeObjectList;
		}
		std::wstring objName = Read::wstring(ifs);
 		makeObjectList.push_back(MakeGameObjectTokey (typeName.c_str())(objName.c_str()));
		GameObject* object = makeObjectList.back().get();
		object->transform.lockUpdate = true;
		if constexpr (DESERIALIZED_VERSION > 0)
		{
			size_t tagsCount = Read::data<size_t>(ifs);
			for (size_t i = 0; i < tagsCount; i++)
			{
				object->SetTag(Read::wstring(ifs));
			}
		}
		object->Active = Read::data<bool>(ifs);
		object->Bounds = Read::BoundingBox(ifs);
		object->Deserialized(ifs);
		//Components
		for (auto& component : object->componentList)
		{
			component->Deserialized(ifs);
		}
		//Transform
		size_t myLevel = Read::data<size_t>(ifs);
		objectTreeMap[myLevel] = &object->transform;
		bool isParent = myLevel > 0;
		if (isParent)
		{
			auto find = objectTreeMap.find(myLevel - 1); //마지막으로 생성된 내 상위 객체를 찾는다
			if (find != objectTreeMap.end())
				object->transform.OnlySetParent(*find->second); //찾으면 부모로 설정
		}
		if (!isParent)
		{
			object->transform._position = Read::Vector3(ifs);
			object->transform._rotation = Read::Quaternion(ifs);
			object->transform._scale	   = Read::Vector3(ifs);
			root = object;

			if (scene)
			{
				scene->loadScenesMap[scene->GetSceneName()].emplace_back(object->GetWeakPtr());
			}
		}
		else
		{
			object->transform._localPosition = Read::Vector3(ifs);
			object->transform._localRotation = Read::Quaternion(ifs);
			object->transform._localScale	= Read::Vector3(ifs);
		}

		//TransformAnimation
		bool isTransformAnimation = Read::data<bool>(ifs);
		if (isTransformAnimation)
		{
			TransformAnimation* animation = &object->AddComponent<TransformAnimation>();
			animation->DeserializedAnimation(ifs);
			transformAnimationVec.push_back(animation);
		}

		//bounds Merged
		if (object->transform.RootParent && object->transform.RootParent == object->transform.Parent)
		{
			if (
				object->Bounds.Extents.x > Mathf::Epsilon &&
				object->Bounds.Extents.y > Mathf::Epsilon &&
				object->Bounds.Extents.z > Mathf::Epsilon
				)
			{
				BoundingBox bd = object->Bounds;
				bd.Transform(bd,
					XMMatrixScaling(object->transform.scale.x, object->transform.scale.y, object->transform.scale.z)
					* XMMatrixRotationQuaternion(object->transform.localRotation)
				);
				object->transform.RootParent->gameObject.Bounds = bd;
			}
		}
	}
}
 