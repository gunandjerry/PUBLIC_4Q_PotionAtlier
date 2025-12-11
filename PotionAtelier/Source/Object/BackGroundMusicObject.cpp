#include "BackGroundMusicObject.h"


BackGroundMusicComponent::BackGroundMusicComponent()
{
	autoPlay = true;
}
BackGroundMusicComponent::~BackGroundMusicComponent() = default;

void BackGroundMusicComponent::Awake()
{
	AudioBankClip::Awake();

}
void BackGroundMusicComponent::Start()
{
	AudioBankClip::Start();
	if (dynamic_cast<BackGroundMusicObject*>(&gameObject))
	{
		static_cast<BackGroundMusicObject*>(&gameObject)->Start();
	}

}

std::atomic<BackGroundMusicObject*> instance = nullptr;
BackGroundMusicObject::BackGroundMusicObject()
{
	component =	&AddComponent<BackGroundMusicComponent>();

}

BackGroundMusicObject::~BackGroundMusicObject()
{

}

void BackGroundMusicObject::Awake()
{
	GameObject::Awake();
	DontDestroyOnLoad(this);
	auto before = instance.exchange(this);
	if (before && before != this)
	{
		before->Name = L"Destroy BackGroundMusic";
		component->bankPath = std::move(before->component->bankPath);
		component->eventName = std::move(before->component->eventName);

		component->bank = std::move(before->component->bank);
		component->eventParametors = std::move(before->component->eventParametors);

		Destroy(before);

	}
	Name = L"BackGroundMusic";
}

void BackGroundMusicObject::Start()
{
	DontDestroyOnLoad(this);
	auto before = instance.exchange(this);	
	if (before && before != this)
	{
		before->Name = L"Destroy BackGroundMusic";
		component->bankPath = std::move(before->component->bankPath);
		component->eventName = std::move(before->component->eventName);

		component->bank = std::move(before->component->bank);
		component->eventParametors = std::move(before->component->eventParametors);


		Destroy(before);
	}
	Name = L"BackGroundMusic";





}
