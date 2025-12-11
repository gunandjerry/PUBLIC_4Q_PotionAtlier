#include "GameManager.h"
#include <framework.h>
#include "Components/GameManagerComponent.h"




GameManagerComponent& GameManager::GetGM()
{
	if (!instance)
	{
		NewGameObject<GameManager>(L"GameManager");
	}
	return *instance->component;
}

GameManagerComponent* GameManager::IsGM()
{
	return instance ? instance->component : nullptr;
}

GameManager::~GameManager()
{
	if (instance == this)
	{
		instance = nullptr;
	}
}

void GameManager::Awake()
{
	if (instance)
	{
		GameObject::Destroy(this);
	}
	else
	{
		instance = this;
		component = &AddComponent<GameManagerComponent>();
		DontDestroyOnLoad(this);
	}
}

GameManagerHelper::GameManagerHelper()
{
	component = &AddComponent<GameManagerHelperComponent>();
}
