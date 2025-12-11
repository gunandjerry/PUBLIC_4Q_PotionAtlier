#pragma once
#include <Core/Transform.h>
#include <Utility/Console.h>
#include <memory>
#include <string>
#include <Manager/GameObjectFactory.h>
#include <Utility/ExceptionUtility.h>

#include <Physics/Struct/CollisionInfo.h>

#include <Core/EventManager.h>


#pragma warning( disable : 4267)

class Component;
class TransformAnimation;
class RenderComponent;
class Rigidbody;
class Collider;
class BoxCollider;
class SphereCollider;
class CapsuleCollider;
class MeshCollider;
class Interactable;

class GameObject
{		 
	SERIALIZED_OBJECT(GameObject)
	friend class Scene;
	friend class SceneManager;
	friend class D3DRenderer;
	friend class PhysicsManager;
	friend class Component;
public:
	GameObject();
	/*이름, 인스턴스 아이디 부여 후 호출되는 함수.*/
	virtual void Awake() {};
	virtual ~GameObject();

	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Serialized(std::ofstream& ofs) {};
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Deserialized(std::ifstream& ifs) {};

	virtual void InspectorImguiDraw();
public:
	static void Destroy(GameObject& obj);
	static void Destroy(GameObject* obj);
	static void DestroyComponent(Component& component);
	static void DestroyComponent(Component* component);
	static void DontDestroyOnLoad(GameObject& obj);
	static void DontDestroyOnLoad(GameObject* obj);
	static GameObject* Find(const wchar_t* name);
	template<typename ObjectType>
	static ObjectType* Find(const wchar_t* name)
	{
		return dynamic_cast<ObjectType*>(Find(name));
	}
	template<class T>
	static T* FindFirst();

	template<typename ObjectType>
	inline static ObjectType* NewGameObject(const wchar_t* name);

	/*동적 할당만*/
	template<typename ObjectType>
	inline static std::shared_ptr<GameObject> MakeGameObject(const wchar_t* name);
public:
	unsigned int GetInstanceID() const { return instanceID; }
	const std::wstring& GetName() const { return name; }
	const std::wstring& SetName(const wchar_t* _name);
	std::string GetNameToString() const;

	bool SetTag(const std::wstring& tag);
	void UnsetTag(const std::wstring& tag)
	{
		auto find = tagSet.find(tag);
		if (find != tagSet.end())
		{
			tagSet.erase(find);
		}
	}
	const std::set<std::wstring> GetTags() const { return tagSet; }
	bool HasTag(const std::wstring& tag) const { return tagSet.find(tag) != tagSet.end(); }

private:
	std::wstring name;
	std::set<std::wstring> tagSet;
	unsigned int instanceID = -1;

public:
	Transform transform;
	bool Active = true;

	/*부모 Active 기준 활성화 여부*/
	bool GetWorldActive() const;
public:
	/*오브젝트 이름. (중복 가능)*/
	_declspec (property(get = GetName, put = SetName)) std::wstring& Name;

	/*컴포넌트 추가*/
	template <typename T>
	T& AddComponent();

	/*TransformAnimation은 하나만 존재 가능.*/
	template <>
	TransformAnimation& AddComponent();

	/*컴포넌트 가져오기*/
	template <typename T>
	T& GetComponent();

	/*컴포넌트 가져오기. (없으면 nullptr 반환)*/
	template <typename T>
	T* IsComponent();

	/*컴포넌트 주소로 인덱스 확인하기. 없으면 -1 반환*/
	int GetComponentIndex(Component* findComponent);

	/*인덱스로 컴포넌트 가져오기. 파라미터로 캐스팅할 컴포넌트 타입을 전달.*/
	template <typename T>
	T* GetComponentAtIndex(int index);

	/*이 오브젝트의 컴포넌트 개수*/
	int GetComponentCount() { return componentList.size(); }

	DirectX::BoundingOrientedBox GetOBBToWorld() const;
	DirectX::BoundingBox GetBBToWorld() const;

	DirectX::BoundingBox Bounds;

	/*카메라 컬링 여부*/
	inline bool IsCameraCulling() const { return isCulling; }

	/*전달 받은 부모까지의 계층 구조를 반환해줍니다.*/
	std::vector<GameObject*> GetHierarchyToParent(GameObject* TargetParent);
	void GetHierarchyToParent(std::vector<GameObject*>& OutVector);
private:
	void Start();
	void FixedUpdate();
	void Update();
	void LateUpdate();
	void Render();
private:
	void EraseComponent(Component* component);
public:
	virtual void OnCollisionEnter(CollisionInfo info) {};
	virtual void OnCollisionStay(CollisionInfo info) {};
	virtual void OnCollisionExit(CollisionInfo info) {};
	virtual void OnTriggerEnter(CollisionInfo info) {};
	virtual void OnTriggerStay(CollisionInfo info) {};
	virtual void OnTriggerExit(CollisionInfo info) {};





private:
	std::vector<std::unique_ptr<Component>> componentList;
	std::vector<RenderComponent*> renderList;
	std::vector<Component*> startList;

	std::vector<Collider*> colliders;
	/* [Warning!!!] Interactable을 한 번에 찾기 위해 임시로 넣어둠 */
	Interactable* interactable{ nullptr };

public:
	std::vector<Collider*>& GetEveryCollider();
	Interactable* _GetInteractableComponent() { return interactable; }


public:
	std::weak_ptr<GameObject> GetWeakPtr() { return myptr; }
private:
	std::weak_ptr<GameObject> myptr;
	bool isCulling = false; //이번 프레임 카메라 컬링 여부

protected:
	class PhysicsActor* physicsActor{ nullptr };
	unsigned int phyiscsLayer{ 0 };
public:
	class PhysicsActor* GetPhysicsActor() { return physicsActor; }
	class PhysicsActor** GetPhysicsActorAddress() { return &physicsActor; }
	void SetPhysicsLayer(unsigned int slot);
	unsigned int GetPhysicsLayer() { return phyiscsLayer; }
};

template<typename T>
inline T& GameObject::AddComponent()
{
	static_assert(std::is_base_of_v<Component, T>, "is not Component");

	T* nComponent = new T;
	nComponent->SetOwner(this);
	nComponent->index = componentList.size();
	nComponent->Awake();
	componentList.emplace_back(nComponent);	
	startList.emplace_back(nComponent); //초기화 항목
	if constexpr (std::is_base_of_v<RenderComponent, T>)
	{
		renderList.push_back(nComponent);
	}
	if constexpr (std::is_base_of_v<Collider, T>)
	{
		colliders.push_back(nComponent);
	}
	if constexpr (std::is_base_of_v<Interactable, T>)
	{
		interactable = static_cast<Interactable*>(nComponent);
	}

	return *nComponent;
}

template<typename T>
inline T& GameObject::GetComponent()
{
	static_assert(std::is_base_of_v<Component, T>, "is not Component");

	for (auto& component : componentList)
	{
		if (typeid(*component) == typeid(T))
		{
			return static_cast<T&>(*component);
		}
	}
	__debugbreak(); //예외) 존재하지 않는 컴포넌트
	throw_GameObject("Exception : Component does not exist", this);
}

template<typename T>
inline T* GameObject::IsComponent()
{
	static_assert(std::is_base_of_v<Component, T>, "is not Component");

	for (auto& component : componentList)
	{
		if (typeid(*component) == typeid(T))
		{
			return static_cast<T*>(component.get());
		}
	}
	return nullptr;
}

template<typename T>
inline T* GameObject::GetComponentAtIndex(int index)
{
	static_assert(std::is_base_of_v<Component, T>, "is not Component");

	if (0 <= index && index < componentList.size())
	{
		T* component = dynamic_cast<T*>(componentList[index].get());
		return component;
	}
	Debug_printf("warrnig : GetComponentAtIndex(int index), index is out of range!\n");
	return nullptr;
}

template<typename ObjectType = GameObject>
ObjectType* NewGameObject(const wchar_t* name)
{
	return GameObject::NewGameObject<ObjectType>(name);
}

template<typename ObjectType = GameObject>
std::shared_ptr<GameObject> MakeGameObject(const wchar_t* name)
{
	return GameObject::MakeGameObject<ObjectType>(name);
}

#include <Manager/InstanceIDManager.h>

template<class T>
inline T* GameObject::FindFirst()
{
	return dynamic_cast<T*>(sceneManager.FindFirstObject<T>());
}


template<typename ObjectType>
inline ObjectType* GameObject::NewGameObject(const wchar_t* name)
{
	static_assert(std::is_base_of_v<GameObject, ObjectType>, "is not gameObject");

	unsigned int id = instanceIDManager.getUniqueID();
	void* pointer = gameObjectFactory.GameObjectAlloc(id);
	if (pointer)
	{
		ObjectType* pObject = reinterpret_cast<ObjectType*>(pointer);
		std::shared_ptr<ObjectType> newObject(pObject, GameObjectFactory::GameObjectDeleter);
		std::shared_ptr<GameObject> baseObject = std::static_pointer_cast<GameObject>(newObject);

		new(pObject)ObjectType();
		baseObject->instanceID = id;
		baseObject->myptr = baseObject;
		baseObject->Name = name;
		baseObject->Awake();

		sceneManager.AddGameObject(baseObject);
		return newObject.get();
	}
	else
	{
		__debugbreak();
		return nullptr;
	}

}

template<typename ObjectType>
inline std::shared_ptr<GameObject> GameObject::MakeGameObject(const wchar_t* name)
{
	static_assert(std::is_base_of_v<GameObject, ObjectType>, "is not gameObject");
	static std::mutex mut;

	mut.lock();
	unsigned int id = instanceIDManager.getUniqueID();
	void* pointer = gameObjectFactory.GameObjectAlloc(id);
	mut.unlock();
	if (pointer)
	{
		ObjectType* pObject = reinterpret_cast<ObjectType*>(pointer);
		std::shared_ptr<ObjectType> newObject(pObject, GameObjectFactory::GameObjectDeleter);
		std::shared_ptr<GameObject> baseObject = std::static_pointer_cast<GameObject>(newObject);

		new(pObject)ObjectType();
		baseObject->instanceID = id;
		baseObject->myptr = baseObject;
		baseObject->Name = name;
		baseObject->Awake();
		return baseObject;
	}
	else
	{
		__debugbreak();
		return nullptr;
	}
}
