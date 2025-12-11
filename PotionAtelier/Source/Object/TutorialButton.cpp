#include "TutorialButton.h"
#include "../Components/TutorialButtonComponent.h"

void TutorialButton::Awake()
{
	AddComponent<UIRenderComponenet2>();
	AddComponent<EventListener>();
	AddComponent<TutorialButtonComponent>();
}
