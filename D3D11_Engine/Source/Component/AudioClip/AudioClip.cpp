#include "Component/AudioClip/AudioClip.h"
#include "Sound/SoundSystem.h"
#include "framework.h"

void AudioClip::SetSound(std::shared_ptr<FMOD::Sound> sound, bool loop)
{
	if (channel != nullptr)
	{
		Stop();
	}

	currentSound = sound;
	if (loop == true)
	{
		currentSound->setMode(FMOD_LOOP_NORMAL);
	}
	else if (loop == false)
	{
		currentSound->setMode(FMOD_LOOP_OFF);
	}
}

void AudioClip::Play()
{
	if (currentSound != nullptr && pause == true)
	{
		Resume();
		return;
	}

	if (currentSound == nullptr) return;

	SoundSystem::GetInstance().GetFMODSystem()->playSound(currentSound.get(), 0, false, &channel);
	channel->setVolume(volume);
}

void AudioClip::Play(std::shared_ptr<FMOD::Sound> sound, bool loop)
{
	SetSound(sound, loop);
	Play();
}

void AudioClip::Pause()
{
	if (currentSound == nullptr || channel == nullptr) return;
	if (pause == true) return;

	channel->setPaused(true);
	pause = true;
}

void AudioClip::Resume()
{
	if (currentSound == nullptr || channel == nullptr) return;
	if (pause == false) return;

	channel->setPaused(false);
	pause = false;
}

void AudioClip::Stop()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->stop();
	channel = nullptr;
	pause = false;
}

void AudioClip::SetVolume(float _volume)
{
	if (volume == _volume) return;

	_volume = _volume > VOLUME_MAX ? VOLUME_MAX : _volume;
	_volume = _volume < 0.0f ? 0.0f : _volume;

	this->volume = _volume;

	if (currentSound == nullptr || channel == nullptr) return;
	channel->setVolume(_volume);
}

void AudioClip::VolumeUp(float amount)
{
	volume = volume + amount > VOLUME_MAX ? VOLUME_MAX : volume + amount;
	
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setVolume(volume);
}

void AudioClip::VolumeDown(float amount)
{
	volume = volume - amount < 0.0f ? 0.0f : volume - amount;

	if (currentSound == nullptr || channel == nullptr) return;

	channel->setVolume(volume);
}

void AudioClip::MuteOn()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setMute(true);
	mute = true;
}

void AudioClip::MuteOff()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setMute(false);
	mute = false;
}

void AudioClip::SetMasterVolume(float master_volume)
{
	SoundSystem::GetInstance().SetMasterVolume(master_volume);
}

bool AudioClip::IsPlaying()
{
	if (currentSound == nullptr || channel == nullptr) return false;

	bool is_playing{ false };
	channel->isPlaying(&is_playing);
	return is_playing;
}
//void AudioClip::SetPitch(float pitch)
//{
//	if (currentSound == nullptr || channel == nullptr) return;
//
//	channel->setPitch(pitch);
//}

AudioBankClip::AudioBankClip()
{
	masterBank = SoundSystem::GetInstance().CreateBank(L"Resource/Sound/Master.bank");
	masterStringsBank = SoundSystem::GetInstance().CreateBank(L"Resource/Sound/Master.strings.bank");
}

AudioBankClip::~AudioBankClip()
{
	if (IsPlaying()) currentSound->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);
}

void AudioBankClip::SetSound(std::shared_ptr<FMOD::Studio::Bank> bank, std::string_view eventName, bool loop)
{
	if (channel != nullptr)
	{
		Stop();
	}

	currentBank = bank;
	FMOD::Studio::EventDescription* findEvent = nullptr;
	{
		auto result = SoundSystem::GetInstance().GetFMODStudioSystem()->getEvent(eventName.data(), &findEvent);
		if (result != FMOD_OK)
		{
			__debugbreak;
			return;
		}
	}


	FMOD::Studio::EventInstance* eventInstance = nullptr;
	{
		auto result = findEvent->createInstance(&eventInstance);
		if (result != FMOD_OK)
		{
			__debugbreak;
			return;
		}
	}
	currentSound = std::shared_ptr<FMOD::Studio::EventInstance>(eventInstance, [](FMOD::Studio::EventInstance* eventInstance)
																{
																	eventInstance->release();
																});
	

}

void AudioBankClip::SetSound()
{
	SetSound(bank, "event:/" + eventName);
}

void AudioBankClip::Play()
{
	if (!currentSound) return;
	if (pause)
	{
		Resume();
		return;
	}
	else
	{
		currentSound->start();
	}

}

void AudioBankClip::Play(std::shared_ptr<FMOD::Studio::Bank>, std::string_view eventName, bool loop)
{
	SetSound(currentBank, eventName, loop);
	Play();
}

void AudioBankClip::Pause()
{
	if (!currentSound) return;
	if (pause) return;
	currentSound->setPaused(true);
	pause = true;
}

void AudioBankClip::Resume()
{
	if (!currentSound) return;
	if (!pause) return;
	currentSound->setPaused(false);
	pause = false;
}

void AudioBankClip::Stop()
{
	if (!currentSound) return;
	currentSound->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);
	pause = false;
}

void AudioBankClip::SetVolume(float _volume)
{
	if (volume == _volume) return;

	_volume = (std::min)(_volume, VOLUME_MAX);
	_volume = (std::max)(_volume, 0.0f);

	this->volume = _volume;

	if (!currentSound) return;
	currentSound->setVolume(volume);
}

void AudioBankClip::VolumeUp(float amount)
{
	SetVolume(volume + amount);
}

void AudioBankClip::VolumeDown(float amount)
{
	SetVolume(volume - amount);
}

void AudioBankClip::MuteOn()
{
	if (!currentSound) return;
	currentSound->setVolume(0.0f);
	mute = true;
}

void AudioBankClip::MuteOff()
{
	if (!currentSound) return;
	currentSound->setVolume(volume);
	mute = false;
}

void AudioBankClip::SetMasterVolume(float master_volume)
{
	SoundSystem::GetInstance().SetMasterVolume(master_volume);
}

bool AudioBankClip::IsPlaying()
{
	if (!currentSound) return false;
}

void AudioBankClip::SetData(std::string_view parameterName, float data)
{
	if (!currentSound) return;
	auto result = currentSound->setParameterByName(parameterName.data(), data);
	if (result != FMOD_OK)
	{
		int a = 8;
	}
}

void AudioBankClip::SetData(std::string_view parameterName, std::string_view data)
{
	if (!currentSound) return;
	auto result = currentSound->setParameterByNameWithLabel(parameterName.data(), data.data());
	if (result != FMOD_OK)
	{
		int a = 8;
	}
}

void AudioBankClip::InspectorImguiDraw()
{
	if (ImGui::TreeNode("AudioClip"))
	{
		if (ImGui::Button("Load"))
		{
			auto path = WinUtility::GetOpenFilePath();
			if (path.size())
			{
				path = std::filesystem::relative(path);
				bank = SoundSystem::GetInstance().CreateBank(path);
				bankPath = path;
			}
		}
		ImGui::SameLine();
		std::string name;
		name.assign(bankPath.begin(), bankPath.end());
		ImGui::Text(name.c_str());
		ImGui::InputText("Event Name", eventName.data(), eventName.size(), ImGuiInputTextFlags_CallbackResize,
						 [](ImGuiInputTextCallbackData* data)
						 {
							 if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
							 {
								 std::string* str = (std::string*)data->UserData;
								 IM_ASSERT(data->Buf == str->c_str());
								 str->resize(data->BufTextLen);
								 data->Buf = str->data();
							 }
							 return 0;
						 }, &eventName);

		if (ImGui::Button("SetSound"))
		{
			SetSound(bank, "event:/" + eventName);
			int count;
			FMOD::Studio::EventDescription* desc;
			if (currentSound)
			{
				currentSound->getDescription(&desc);
				desc->getParameterDescriptionCount(&count);
				eventParametors.clear();
				for (size_t i = 0; i < count; i++)
				{
					FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
					desc->getParameterDescriptionByIndex(i, &paramDesc);
					BankEventParametor param{};
					param.name = paramDesc.name;
					param.isString = paramDesc.type == FMOD_STUDIO_PARAMETER_GAME_CONTROLLED;

					eventParametors.emplace_back(param);
				}
			}

		}

		for (size_t i = 0; i < eventParametors.size(); i++)
		{
			eventParametors[i].InspectorImguiDraw();
		}
		if (ImGui::Button("Play"))
		{
			Play();
			for (auto& param : eventParametors)
			{
				if (param.isString)
				{
					SetData(param.name, param.value);
				}
				else
				{
					SetData(param.name, param.floatValue);
				}
			}
		}
		if (ImGui::Button("Pause"))
		{
			Pause();
		}
		ImGui::Checkbox("AutoPlay", &autoPlay);
		ImGui::TreePop();
	}
}


void AudioBankClip::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, 2);
	Binary::Write::wstring(ofs, bankPath);
	Binary::Write::string(ofs, eventName);

	Binary::Write::data(ofs, eventParametors.size());
	for (auto& param : eventParametors)
	{
		Binary::Write::string(ofs, param.name);
		Binary::Write::data(ofs, param.isString);
		if (param.isString)
		{
			Binary::Write::string(ofs, param.value);
		}
		else
		{
			Binary::Write::data(ofs, param.floatValue);
		}
	}

	// version 2
	Binary::Write::data(ofs, autoPlay);
	// ~version 2
	
}

void AudioBankClip::Deserialized(std::ifstream& ifs)
{
	uint32_t version = Binary::Read::data<uint32_t>(ifs);
	bankPath = Binary::Read::wstring(ifs);
	eventName = Binary::Read::string(ifs);


	size_t size = Binary::Read::data<size_t>(ifs);
	eventParametors.resize(size);

	bank = SoundSystem::GetInstance().CreateBank(bankPath);
	SetSound(bank, "event:/" + eventName);

	for (auto& param : eventParametors)
	{
		param.name = Binary::Read::string(ifs);
		param.isString = Binary::Read::data<bool>(ifs);
		if (param.isString)
		{
			param.value = Binary::Read::string(ifs);
			SetData(param.name, param.value);
		}
		else
		{
			param.floatValue = Binary::Read::data<float>(ifs);
			SetData(param.name, param.floatValue);
		}
	}
	if (version >= 2)
	{
		autoPlay = Binary::Read::data<bool>(ifs);
	}

}

void AudioBankClip::Awake()
{



}

void AudioBankClip::Start()
{
	bank = SoundSystem::GetInstance().CreateBank(bankPath);

	SetSound(bank, "event:/" + eventName);

	for (auto& param : eventParametors)
	{
		if (param.isString)
		{
			SetData(param.name, param.value);
		}
		else
		{
			SetData(param.name, param.floatValue);
		}
	}

	if (autoPlay) Play();
}

void BankEventParametor::InspectorImguiDraw()
{
	ImGui::BeginChild(std::format("BankEventParametor {}", name.data()).c_str(), ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	
	if (isString)
	{
		ImGui::InputText(name.data(), value.data(), value.size() + 1, ImGuiInputTextFlags_CallbackResize,
						 [](ImGuiInputTextCallbackData* data)
						 {
							 if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
							 {
								 std::string* str = (std::string*)data->UserData;
								 IM_ASSERT(data->Buf == str->c_str());
								 str->resize(data->BufTextLen);
								 data->Buf = str->data();
							 }
							 return 0;
						 }, &value);
	}
	else
	{
		ImGui::InputFloat(name.data(), (float*)&floatValue);
	}
	ImGui::EndChild();
}