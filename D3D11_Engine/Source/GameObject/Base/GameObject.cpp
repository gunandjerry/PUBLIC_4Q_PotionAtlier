#include "GameObject.h"	   
#include <Utility/Console.h>

#include <Component/Animation/TransformAnimation.h>
#include <Component/Camera/Camera.h>
#include <Component/Base/RenderComponent.h>
#include <Manager/SceneManager.h>
#include <Manager/InstanceIDManager.h>
#include <Utility/utfConvert.h>
#include <Utility/ImguiHelper.h>
#include <Math/Mathf.h>

#include <Physics/PhysicsManager.h>
#include <Physics/PhysicsActor/PhysicsActor.h>
#include <Component/Rigidbody/Rigidbody.h>
#include <Component/Collider/Collider.h>
#include <Component/Collider/BoxCollider.h>
#include <Component/Collider/SphereCollider.h>
#include <Component/Collider/CapsuleCollider.h>
#include <Component/Collider/MeshCollider.h>

void GameObject::InspectorImguiDraw()
{
	for (auto& component : componentList)
	{
		component->InspectorImguiDraw();
	}
}

void GameObject::Destroy(GameObject& obj)
{
	sceneManager.DestroyObject(obj);
}

void GameObject::Destroy(GameObject* obj)
{
	sceneManager.DestroyObject(obj);
}

void GameObject::DontDestroyOnLoad(GameObject& obj)
{
	sceneManager.DontDestroyOnLoad(obj);
}

void GameObject::DontDestroyOnLoad(GameObject* obj)
{
	sceneManager.DontDestroyOnLoad(obj);
}

GameObject* GameObject::Find(const wchar_t* name)
{
	return sceneManager.FindObject(name);
}

GameObject::GameObject()
{
	transform._gameObject = this;	 
	Bounds.Extents = { 0,0,0 };
}

GameObject::~GameObject()
{
	//Debug_printf("%s, destroy\n", Name);
	if (Scene::GuizmoSetting.SelectObject == this)
	{
		Scene::GuizmoSetting.SelectObject = nullptr;
	}
	instanceIDManager.returnID(instanceID);

	componentList.clear();
	if (physicsActor) delete physicsActor;
}

template <>
TransformAnimation& GameObject::AddComponent()
{
	GameObject* obj = transform.rootParent ? &transform.rootParent->gameObject : this;
	if (!obj->IsComponent<TransformAnimation>())
	{
		TransformAnimation* nComponent = new TransformAnimation;
		nComponent->SetOwner(obj);
		nComponent->index = componentList.size();
		nComponent->Awake();
		componentList.emplace_back(nComponent);
		startList.emplace_back(nComponent); //초기화 항목
		return *nComponent;
	}
	else
	{
		__debugbreak();
		throw_GameObject("Error : Only one TransformAnimation component can be used.", this);
	}
}

std::vector<Collider*>& GameObject::GetEveryCollider()
{
	return colliders;
}

int GameObject::GetComponentIndex(Component* findComponent)
{
	for (size_t i = 0; i < componentList.size(); i++)
	{
		if (componentList[i].get() == findComponent)
		{
			return i;
		}
	}
	return -1;
}

void GameObject::DestroyComponent(Component& component)
{
	DestroyComponent(&component);
}

void GameObject::DestroyComponent(Component* component)
{
	sceneManager.eraseComponentSet.insert(component);
}

DirectX::BoundingOrientedBox GameObject::GetOBBToWorld() const
{
	//OBB

	BoundingOrientedBox boundingBoxOut;
	boundingBoxOut.CreateFromBoundingBox(boundingBoxOut, Bounds);
	boundingBoxOut.Transform(boundingBoxOut, transform.GetWM());

	return boundingBoxOut;
}

DirectX::BoundingBox GameObject::GetBBToWorld() const
{
	//BB
	BoundingBox boundingBoxOut = Bounds;
	boundingBoxOut.Transform(boundingBoxOut, transform.GetWM());
	return boundingBoxOut;
}

std::vector<GameObject*> GameObject::GetHierarchyToParent(GameObject* TargetParent)
{
	std::vector<GameObject*> hierarchy;
	Transform* curr = &transform;
	if (&TargetParent->transform == curr)
	{
		hierarchy.push_back(this);
		return hierarchy;
	}
	while (&TargetParent->transform != curr)
	{
		if (curr->parent == nullptr)
		{
			hierarchy.clear();
			return hierarchy;
		}
		hierarchy.push_back(&curr->gameObject);
		curr = curr->parent;
	}
	hierarchy.push_back(&curr->gameObject);
	std::reverse(hierarchy.begin(), hierarchy.end());
	return hierarchy;
}

void GameObject::GetHierarchyToParent(std::vector<GameObject*>& OutVector)
{
	Transform* curr = &transform;

	while (true)
	{
		if (curr->parent == nullptr)
		{
			return;
		}
		OutVector.push_back(&curr->gameObject);
		curr = curr->parent;
	}

}

void GameObject::Start()
{
	if (!startList.empty())
	{
		static std::vector<Component*> tempStartList;
		tempStartList.clear();
		std::ranges::copy_if(startList, std::back_inserter(tempStartList),
			[](Component* component)
			{
				return component->Enable;
			});
		for (auto& component : tempStartList)
		{
			component->Start();
			component->isStart = true;
		}
		std::erase_if(startList
			,[](auto& component)
			{ 
				return component->isStart; 
			});
	}
}

void GameObject::FixedUpdate()
{
	for (auto& component : componentList)
	{
		if(component->Enable)
			component->FixedUpdate();
	}
}

void GameObject::Update()
{
	for (auto& component : componentList)
	{
		if (component->Enable)
			component->Update();
	}
}

void GameObject::LateUpdate()
{
	for (auto& component : componentList)
	{
		if (component->Enable)
			component->LateUpdate();
	}
}

void GameObject::Render()
{	
	for (auto& component : renderList)
	{
		if (component->Enable)
			component->Render();
	}
}

void GameObject::EraseComponent(Component* component)
{
	int size = (int)componentList.size();
	int comIndex = 0;
	for (auto& com : componentList)
	{
		if (typeid(*com) == typeid(*component))
			break;

		++comIndex;
	}
	int renderIndex = 0;
	if (comIndex < size)
	{
		size = renderList.size();
		for (auto& com : renderList)
		{
			if (typeid(*com) == typeid(*component))
				break;

			renderIndex++;
		}
		componentList.erase((componentList.begin() + comIndex));
		if (renderIndex < size)
			renderList.erase((renderList.begin() + renderIndex));
	}
	int colliderIndex = 0;
	if (comIndex < size)
	{
		size = colliders.size();
		for (auto& com : colliders)
		{
			if (typeid(*com) == typeid(*component))
				break;

			colliderIndex++;
		}
		if (renderIndex < size)
			colliders.erase((colliders.begin() + colliderIndex));
	}
}

void GameObject::SetPhysicsLayer(unsigned int slot)
{
	PhysicsManager::GetInstance().SetPhysicsLayer(this, slot);
}

const std::wstring& GameObject::SetName(const wchar_t* _name)
{
	if (wcscmp(name.c_str(), _name) == 0)
		return name;

	name = sceneManager.ChangeObjectName(this->instanceID, name, _name);
	return name;
}

std::string GameObject::GetNameToString() const
{
	return utfConvert::wstring_to_utf8(GetName());
}

bool GameObject::SetTag(const std::wstring& tag)
{
	auto [iter, result] = tagSet.insert(tag);
	return result;
}

bool GameObject::GetWorldActive() const
{
	if (transform.parent)
	{
		const Transform* current = &this->transform;
		while (current != this->transform.rootParent)
		{
			if (current->gameObject.Active == false)
				return false;

			current = current->parent;
		}
		return current->gameObject.Active == false ? false : Active;
	}
	else
	{
		return Active;
	}
}
