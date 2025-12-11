#pragma once
#include <GameObject/Base/GameObject.h>
#include "framework.h"

class AudioPlayerObject : public GameObject
{
	SERIALIZED_OBJECT(AudioPlayerObject);
public:
	AudioPlayerObject();
	virtual ~AudioPlayerObject() override;

	void Awake() override;
	void Start();
	class AudioBankClip& component;
};

