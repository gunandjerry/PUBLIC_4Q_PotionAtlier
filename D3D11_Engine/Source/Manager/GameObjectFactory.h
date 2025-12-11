#pragma once
#include <Core/TSingleton.h>
#include <Manager/SceneManager.h>
#include <map>
#include <string>
#include <functional>
#include <future>
#include <memory>
#include <Core/StaticBlockMemoryPool.h>

/*오브젝트 클래스 선언시 다음 매크로 포함*/
#define SERIALIZED_OBJECT(TypeName)	\
friend class GameObjectFactory;	\
inline static bool TypeName##FactoryInit = GameObjectFactory::AddNewObjectFuntion<TypeName>(__FILE__); \

class GameObject;
class GameObjectFactory;
extern GameObjectFactory& gameObjectFactory;

class GameObjectFactory : public TSingleton<GameObjectFactory>
{
	using NewObjectMapType = std::map<std::string, std::function<GameObject* (const wchar_t* name)>>;
	using ConstructorMapType = std::map<std::string, std::function<std::shared_ptr<GameObject> (const wchar_t* name)>>;
	template <typename T>
	friend class TSingleton;
	friend class D3D11_GameApp;
	inline static size_t MaxGameObjectClassSize = 0;
	inline static std::string MaxSizeGameObjectClassName;
public:
	//게임 오브젝트 커스텀 딜리터
	static void GameObjectDeleter(GameObject* pObj);

	template<typename T>
	static bool AddNewObjectFuntion(const char* filePath)
	{
		static_assert(std::is_base_of_v<GameObject, T>, "T is not gameObject");
		std::string key = typeid(T).name();

		NewObjectMapType& objectFuncMap = const_cast<NewObjectMapType&>(GetNewGameObjectFuncMap());
		auto func = [](const wchar_t* name) { return NewGameObject<T>(name); };
		objectFuncMap[key] = func;
		RegisterParentFolder(key.c_str(), filePath);

		ConstructorMapType& constructorMap = const_cast<ConstructorMapType&>(GetGameObjectConstructorMap());
		auto makeFunc = [](const wchar_t* name){ return MakeGameObject<T>(name); };
		constructorMap[key] = makeFunc;

		if (MaxGameObjectClassSize < sizeof(T))
		{
			MaxGameObjectClassSize = sizeof(T);
			MaxSizeGameObjectClassName = key;
		}
		return true;
	}
	static void RegisterParentFolder(const char* key, const char* filePath);

	static const std::map<std::string, std::function<GameObject* (const wchar_t* name)>>& GetNewGameObjectFuncMap();
	static const ConstructorMapType& GetGameObjectConstructorMap();
	static const std::map<std::string, std::string>& GetGameObjectParentFolderMap();
private:
	GameObjectFactory() = default;
	~GameObjectFactory() override;

	StaticBlockMemoryPool gameObjectMemoryPool;
public:
	void InitializeMemoryPool();
	void UninitializeMemoryPool();
	void CompactObjectMemoryPool();

	void* GameObjectAlloc(size_t id);
	std::function<GameObject*(const wchar_t* name)>& NewGameObjectToKey(const char* key);
	std::function<std::shared_ptr<GameObject>(const wchar_t* name)> MakeGameObjectTokey(const char* key);
	const char* GetObjectParentFolder(const char* key);

	void SerializedScene(Scene* scene, const wchar_t* WritePath, bool isOverride = false);
	void DeserializedScene(Scene* scene, const wchar_t* ReadPath);

	inline static std::mutex deserializedMutex;
	std::vector<std::thread> deserializedThreads;
	/*비동기 직렬화 스레드 대기 후 정리*/
	void  DeserializedThreadsAsync();

	void SerializedObject(GameObject* object, const wchar_t* WritePath, bool isOverride = false);
	GameObject* DeserializedObject(const wchar_t* ReadPath);
	void DeserializedObjectAsync(const wchar_t* ReadPath, const std::function<void(GameObject* object)>& endCallBackFunc);

private:
	void Serialized(GameObject* object, std::ofstream& ofs, size_t level);
	std::vector<std::shared_ptr<GameObject>> Deserialized(std::ifstream& ifs, Scene* scene = nullptr);
};

