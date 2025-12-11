#pragma once
#include "GameObject/Base/GameObject.h"
#include "Components/RecipeManagerComponent.h"

class RecipeManager : public GameObject
{
	SERIALIZED_OBJECT(RecipeManager);
public:
	static class RecipeManagerComponent* GetInstance();

	void Awake() override;
};