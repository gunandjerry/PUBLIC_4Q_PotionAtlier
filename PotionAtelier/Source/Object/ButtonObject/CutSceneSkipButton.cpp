#include "CutSceneSkipButton.h"
#include "Core/TimeSystem.h"
#include "Object/GameManager.h"
#include "Components/GameManagerComponent.h"
#include "Components/BoingBoing.h"
#include "Object\AudioPlayerObject.h"
#include "Object/CutSceneManagerObject.h"

CutSceneSkipButton::CutSceneSkipButton() :
	boingboing(AddComponent<BoingBoingUI>())
{
	boingboing.use_on_ui_render2 = true;

}

void CutSceneSkipButton::Awake()
{
	eventListener.SetOnClickDown(
		[this]
		{
			if (const auto& findItem = GameObject::FindFirst<CutSceneManagerObject>())
			{
				findItem->Skip();
			}
			eventListener.SetOnClickDown([]() {}); // Disable further clicks



		});
}



