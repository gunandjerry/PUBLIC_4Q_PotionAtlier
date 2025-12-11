#include "SqueezerObject.h"
#include <Components/SqueezerCooker.h>
#include <Components/GridPlacementSystem.h>
#include <Components/BoingBoing.h>

void SqueezerObject::Awake()
{
	AddComponent<BoxCollider>();
	AddComponent<SqueezerCooker>();
	AddComponent<GridPlacementSystem>();
	AddComponent<BoingBoing>();
}
