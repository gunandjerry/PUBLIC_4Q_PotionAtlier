#pragma once
#include <memory>

class Transform;
class GameObject;
class Component
{
	friend class GameObject;
public:
	Component();
	virtual ~Component();
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Serialized(std::ofstream& ofs) {};
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Deserialized(std::ifstream& ifs) {};
	virtual void InspectorImguiDraw() {}
public:
	/*컴포넌트 Owner 등록 후 바로 호출되는 함수 */
	virtual void Awake()		= 0; //초기화 함수는 명시적으로 호출 가능.

	/*컴포넌트 첫 Update 호출 전에 단 한번 호출해주는 함수.*/
	virtual void Start() {}; //초기화 함수는 명시적으로 호출 가능.
protected:
	virtual void FixedUpdate()  = 0;
	virtual void Update()		= 0;
	virtual void LateUpdate()	= 0;

private:
	GameObject* _gameObject = nullptr;
	void SetOwner(GameObject* gameObject);
	int index = -1;
	bool isStart = false;

public:
	bool Enable = true;

public:
	GameObject& GetGameObject() const;
	_declspec (property(get = GetGameObject)) GameObject& gameObject;

	Transform& GetTransform() const;
	Transform& SetTransform(const Transform& tr) const;
	__declspec (property(get = GetTransform, put = SetTransform)) Transform& transform;

	template <typename T>
	T& AddComponent();

	template <typename T>
	T& GetComponent();

	template <typename T>
	T* IsComponent();

	int GetComponentIndex() const { return index; }
};

#include <GameObject\Base\GameObject.h>
template<typename T>
inline T& Component::AddComponent()
{
	return gameObject.AddComponent<T>();
}

template<typename T>
inline T& Component::GetComponent()
{
	return gameObject.GetComponent<T>();
}

template<typename T>
inline T* Component::IsComponent()
{
	return gameObject.IsComponent<T>();
}
