#pragma once
#include <memory>
#include <map>
#include <string>
#include <Core/TSingleton.h>
#include <Sound/FMODFramework.h>

class SoundManager : public TSingleton<SoundManager>
{
	friend class TSingleton<SoundManager>;

	std::map<std::wstring, std::shared_ptr<FMOD::Sound>> sounds;

public:
	std::shared_ptr<FMOD::Sound> AddSound(std::wstring_view key, std::wstring_view path);
	std::shared_ptr<FMOD::Sound> GetSound(std::wstring_view key);
	void RemoveSound(std::wstring_view key);

private:
	SoundManager() {}
	~SoundManager();
};