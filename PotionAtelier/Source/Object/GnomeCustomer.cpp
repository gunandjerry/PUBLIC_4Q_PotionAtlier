#include "GnomeCustomer.h"
#include <Components/CustomerAI.h>
#include <Component/CharacterController/CharacterController.h>
#include "../Components/BoingBoing.h"
#include "Components/FlyingGnome.h"
#include "Components/TextBubble.h"

void GnomeCustomer::Awake()
{
	AddComponent<BoingBoing>();
	AddComponent<CharacterController>();
	AddComponent<CustomerAI>();
	AddComponent<FlyingGnome>();

	auto& bubble = AddComponent<TextBubble>();
	bubble.type = TextBubbleType::Gnome;
}
