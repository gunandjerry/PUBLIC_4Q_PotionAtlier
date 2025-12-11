#include "Player.h"
#include "../Components/PlayerController.h"
#include "../Components/Holding.h"
#include "../Components/BoingBoing.h"
#include "../Components/FlyingGnome.h"
#include "../Components/TextBubble.h"

#include "../StringResource.h"

void Player::Awake()
{
	AddComponent<CharacterController>();
	
	AddComponent<PlayerController>();

	AddComponent<BoingBoing>();
	AddComponent<TextBubble>();
}
