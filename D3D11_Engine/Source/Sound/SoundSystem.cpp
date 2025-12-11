#include <filesystem>
#include "Sound/SoundSystem.h"
#include "Manager/SoundManager.h"

SoundSystem::SoundSystem()
{
	FMOD_RESULT fmod_result;
	fmod_result = FMOD::System_Create(&fmod_system);
	if (fmod_result != FMOD_OK) __debugbreak;
	fmod_result = fmod_system->getVersion(&fmod_version);
	if (fmod_result != FMOD_OK) __debugbreak;
	fmod_result = fmod_system->init(fmod_channel_count, FMOD_INIT_NORMAL, fmod_extra_driver_data);
	if (fmod_result != FMOD_OK) __debugbreak;

	fmod_system->getMasterChannelGroup(&master_group);


	fmod_result = FMOD::Studio::System::create(&fmod_Studiosystem);
	if (fmod_result == FMOD_OK) __debugbreak;
	fmod_result = fmod_Studiosystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
	if (fmod_result == FMOD_OK) __debugbreak;


}

SoundSystem::~SoundSystem()
{
	if (fmod_system)
	{
		fmod_system->close();
		fmod_system->release();
		fmod_system = nullptr;
	}

	if (fmod_Studiosystem)
	{
		fmod_Studiosystem->unloadAll();
		fmod_Studiosystem->flushCommands();
		fmod_Studiosystem->release();
	}


}

void SoundSystem::UpdateFMODSystem()
{
	fmod_system->update();
	fmod_Studiosystem->update();
}

void SoundSystem::SetMasterVolume(float volume)
{
	master_group->setVolume(volume);
}

std::shared_ptr<FMOD::Sound> SoundSystem::CreateSound(std::wstring_view path)
{
	std::filesystem::path p(path);
	if (std::filesystem::exists(p) == false)
	{
		__debugbreak;
		return nullptr;
	}

	std::string _path;
	_path.assign(path.begin(), path.end());

	FMOD::Sound* sound{ nullptr };
	FMOD_RESULT fr = fmod_system->createSound(_path.c_str(), FMOD_DEFAULT, 0, &sound);

	if (fr != FMOD_RESULT::FMOD_OK)
	{
		__debugbreak;
		return nullptr;
	}

	return std::shared_ptr<FMOD::Sound>(sound, [](FMOD::Sound* sound)
										{
											sound->release();
										});
}

std::shared_ptr<FMOD::Studio::Bank> SoundSystem::CreateBank(std::wstring_view path)
{

	if (auto findIter = banks.find(path.data()); findIter == banks.end() || findIter->second.expired())
	{
		FMOD::Studio::Bank* newBank{ nullptr };
		std::filesystem::path _path = std::filesystem::relative(path);
		auto fr = fmod_Studiosystem->loadBankFile(_path.string().c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &newBank);
		if (fr != FMOD_RESULT::FMOD_OK)
		{
			__debugbreak;
			return nullptr;
		}
		auto _bank = std::shared_ptr<FMOD::Studio::Bank>(newBank, [](FMOD::Studio::Bank* newBank)
														 {
															 newBank->unload();
														 });
		banks[path.data()] = _bank;
		return _bank;
	}
	else
	{
		return banks[path.data()].lock();
	}


}
