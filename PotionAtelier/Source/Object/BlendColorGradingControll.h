#pragma once

#include "framework.h"

class BlendColorGradingControllComponent : public Component
{
public:
	virtual void Awake() override;
	virtual void Start() override;

	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);

	virtual void InspectorImguiDraw();
	

	class GameManagerComponent* gameManager{ nullptr };
	class PostProcessComponent* postProcess;
protected:
	virtual void FixedUpdate() override {}
	virtual void LateUpdate() override {}

	virtual void Update() override;
	
	float elapsedTime = 0;

	Texture texture[3];
	Texture originTexture[3];
	std::wstring texturePath[3];
};


class BlendColorGradingControll : public GameObject
{
	SERIALIZED_OBJECT(BlendColorGradingControll)
	

public:
	BlendColorGradingControll()
	{
		AddComponent<BlendColorGradingControllComponent>();
	}

private:

};
