#pragma once
#include "Sound/FMODFramework.h"
#include "Component/Base/Component.h"

/**
 * @brief Fmod기본 사운드
 */
class AudioClip : public Component
{
	static constexpr float VOLUME_MAX = 100.0f;

	FMOD::Channel* channel{ nullptr };
	std::shared_ptr<FMOD::Sound> currentSound{ nullptr };
	float volume{ 1.0f };

	bool pause{ false };
	bool mute{ false };

public:
	void SetSound(std::shared_ptr<FMOD::Sound>, bool loop = false);

	void Play();
	void Play(std::shared_ptr<FMOD::Sound>, bool loop = false);
	
	void Pause();
	void Resume();
	void Stop();

	void SetVolume(float volume);
	float GetVolume() { return volume; }
	void VolumeUp(float amount);
	void VolumeDown(float amount);
	void MuteOn();
	void MuteOff();

	void SetMasterVolume(float master_volume);
	bool IsPlaying();

	//void SetPitch(float pitch);




public:
	virtual void Awake() override {}
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}
};

struct BankEventParametor
{
	std::string name;
	bool isString;
	std::string value;
	float floatValue;

	void InspectorImguiDraw();

};


/**
 * @brief 사운드 뱅크 사운드
 */
class AudioBankClip : public Component
{
public:
	AudioBankClip();
	virtual ~AudioBankClip() override;

public:
	void SetSound(std::shared_ptr<FMOD::Studio::Bank>, std::string_view eventName, bool loop = false);
	void SetSound();

	void Play();
	void Play(std::shared_ptr<FMOD::Studio::Bank>, std::string_view eventName, bool loop = false);

	void Pause();
	void Resume();
	void Stop();

	void SetVolume(float volume);
	float GetVolume() { return volume; }
	void VolumeUp(float amount);
	void VolumeDown(float amount);
	void MuteOn();
	void MuteOff();

	void SetMasterVolume(float master_volume);
	bool IsPlaying();


	void SetData(std::string_view parameterName, float data);
	void SetData(std::string_view parameterName, std::string_view data);
	//void SetPitch(float pitch);


	virtual void InspectorImguiDraw();
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;


public:
	virtual void Awake() override;
	virtual void Start() override;
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}

	static constexpr float VOLUME_MAX = 100.0f;

	FMOD::Channel* channel{ nullptr };
	std::shared_ptr<FMOD::Studio::EventInstance> currentSound{ nullptr };
	std::shared_ptr<FMOD::Studio::Bank> currentBank{ nullptr };
	float volume{ 1.0f };

	bool pause{ false };
	bool mute{ false };

	std::shared_ptr<FMOD::Studio::Bank> masterBank;
	std::shared_ptr<FMOD::Studio::Bank> masterStringsBank;
public:

	std::wstring bankPath;
	std::string eventName;

	std::shared_ptr<FMOD::Studio::Bank> bank;
	std::vector<BankEventParametor> eventParametors;
	bool autoPlay = false; // 백그라운드 전용인데 직렬화가 여기밖에안되서 여기에 넣음
};


using Sound = FMOD::Sound;

