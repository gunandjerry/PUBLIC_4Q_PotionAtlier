#pragma once
#include "GameObject/Base/GameObject.h"
#include "Components/TutorialManagerComponent.h"

class TutorialManager : public GameObject
{
	SERIALIZED_OBJECT(TutorialManager);
public:
	static TutorialManagerComponent* GetInstance();
	~TutorialManager();

	void Awake() override;
private:
	inline static TutorialManager* instance = nullptr;
	TutorialManagerComponent* component = nullptr;
};