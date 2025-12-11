#pragma once
#include "Sound/FMODFramework.h"
#include "Core/TSingleton.h"
#include <memory>
#include <string>
#include <map>

class SoundSystem : public TSingleton<SoundSystem>
{
	FMOD::System* fmod_system{ nullptr };
	FMOD::Studio::System* fmod_Studiosystem{ nullptr };
	FMOD::ChannelGroup* master_group{ nullptr };
	unsigned int fmod_channel_count{ 64 };
	unsigned int fmod_version;
	void* fmod_extra_driver_data{ nullptr };

	std::map<std::wstring, std::weak_ptr<FMOD::Studio::Bank>> banks;
public:
	SoundSystem();
	virtual ~SoundSystem();
	void UpdateFMODSystem();

	// ±âº»°ª 1.0(100%),
	void SetMasterVolume(float volume);

	std::shared_ptr<FMOD::Sound> CreateSound(std::wstring_view path);
	std::shared_ptr<FMOD::Studio::Bank> CreateBank(std::wstring_view path);

	FMOD::System* GetFMODSystem() { return fmod_system; }
	auto GetFMODStudioSystem() { return fmod_Studiosystem; }
};

