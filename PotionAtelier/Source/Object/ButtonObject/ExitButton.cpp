#include "ExitButton.h"
#include "Core/TimeSystem.h"
#include "Object/GameManager.h"
#include "Components/GameManagerComponent.h"
#include "Components/BoingBoing.h"
#include "Object\AudioPlayerObject.h"

ExitButton::ExitButton():
	boingboing(AddComponent<BoingBoingUI>())
{
	boingboing.use_on_ui_render2 = true;
}

void ExitButton::Awake()
{
	eventListener.SetOnClickDown(
		[this]
		{
			boingboing.Boing(0);
			TimeSystem::Time.DelayedInvok(
				[]
				{
					if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Click"))
					{
						findItem->IsComponent<AudioBankClip>()->Play();
					}

					D3D11_GameApp::GameEnd();
				},
				0.7f);		
		});
}
