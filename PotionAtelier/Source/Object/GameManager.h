#pragma once
#include <GameObject/Base/GameObject.h>
#include <Components/GameManagerComponent.h>

class GameManagerHelper : public GameObject
{
	SERIALIZED_OBJECT(GameManagerHelper);
public:
	GameManagerHelper();
	GameManagerHelperComponent* component = nullptr;
};

class GameManager : public GameObject
{
	SERIALIZED_OBJECT(GameManager);
public:
	static GameManagerComponent& GetGM();
	static GameManagerComponent* IsGM();
	virtual ~GameManager() override;

	void Awake() override;
private:
	inline static GameManager* instance = nullptr;
	GameManagerComponent* component = nullptr;
};