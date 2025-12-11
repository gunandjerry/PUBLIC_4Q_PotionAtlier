#include "Manager/SoundManager.h"
#include <Sound/SoundSystem.h>

std::shared_ptr<FMOD::Sound> SoundManager::AddSound(std::wstring_view key, std::wstring_view path)
{
	if (sounds.find(key.data()) == sounds.end())
	{
		auto _sound = SoundSystem::GetInstance().CreateSound(path);
		sounds[key.data()] = _sound;
		return _sound;
	}
	else
	{
		printf("Resource already exists : FMOD::Sound / %s\n", key.data());
		return sounds[key.data()];
	}
}

std::shared_ptr<FMOD::Sound> SoundManager::GetSound(std::wstring_view key)
{
	if (auto findIter = sounds.find(key.data());
		findIter != sounds.end())
	{
		return findIter->second;
	}
	else
	{
		printf("Resource not found : FMOD::Sound / %s\n", key.data());
		return nullptr;
	}
}

void SoundManager::RemoveSound(std::wstring_view key)
{
	if (auto findIter = sounds.find(key.data()); 
		findIter != sounds.end())
	{
		sounds.erase(findIter);
		return;
	}
}

SoundManager::~SoundManager()
{
}