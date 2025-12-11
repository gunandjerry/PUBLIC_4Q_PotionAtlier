#pragma once
#include <GameObject/Base/GameObject.h>
#include "framework.h"

#include <vector>
#include <string>

class BackGroundMusicComponent : public AudioBankClip
{
public:
	BackGroundMusicComponent();
	virtual ~BackGroundMusicComponent() override;

	virtual void Awake() override;
	virtual void Start() override;
};

class BackGroundMusicObject : public GameObject
{
	SERIALIZED_OBJECT(BackGroundMusicObject);
public:
	BackGroundMusicObject();
	virtual ~BackGroundMusicObject() override;

	void Awake() override;
	void Start();
	class BackGroundMusicComponent* component = nullptr;
};

