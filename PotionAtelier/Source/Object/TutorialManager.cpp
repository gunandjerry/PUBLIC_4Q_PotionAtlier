#include "TutorialManager.h"

TutorialManagerComponent* TutorialManager::GetInstance()
{
	if (!instance)
	{
		instance = NewGameObject<TutorialManager>(L"TutorialManager");
		instance->Active = true;
	}
	return instance->component;
}

TutorialManager::~TutorialManager()
{
	if (instance == this)
	{
		instance = nullptr;
	}
}

void TutorialManager::Awake()
{
	if (instance)
	{
		GameObject::Destroy(this);
	}
	else
	{
		instance = this;
		component = &AddComponent<TutorialManagerComponent>();
	}
}
