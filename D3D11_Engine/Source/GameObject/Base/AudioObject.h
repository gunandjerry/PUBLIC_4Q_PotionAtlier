#pragma once
#include <GameObject/Base/GameObject.h>
#include <Component/AudioClip/AudioClip.h>


class AudioObject : public GameObject
{
	SERIALIZED_OBJECT(AudioObject)
public:
	AudioObject();
	virtual ~AudioObject() override = default;
	AudioClip& audioClip;
};



class AudiBankObject : public GameObject
{
	SERIALIZED_OBJECT(AudiBankObject)
public:
	AudiBankObject();
	virtual ~AudiBankObject() override = default;
	AudioBankClip& audioClip;
};

