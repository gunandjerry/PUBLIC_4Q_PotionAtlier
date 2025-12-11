#include "RecipeManager.h"

RecipeManagerComponent* RecipeManager::GetInstance()
{
	return &Find(L"RecipeManager")->GetComponent<RecipeManagerComponent>();
}

void RecipeManager::Awake()
{
	AddComponent<RecipeManagerComponent>();
}