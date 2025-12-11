#include "AudioPlayerObject.h"

AudioPlayerObject::AudioPlayerObject() : 
	component(AddComponent<AudioBankClip>())
{

}

AudioPlayerObject::~AudioPlayerObject()
{

}

void AudioPlayerObject::Awake()
{
}

void AudioPlayerObject::Start()
{
}
