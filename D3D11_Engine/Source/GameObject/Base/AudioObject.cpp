#include "AudioObject.h"

AudioObject::AudioObject() :
	audioClip(AddComponent<AudioClip>())
{
}

AudiBankObject::AudiBankObject() :
	audioClip(AddComponent<AudioBankClip>())
{
}
