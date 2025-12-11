#include "HandMillObject.h"
#include <Components/HandMillCooker.h>
#include <Components/GridPlacementSystem.h>
#include <Components/BoingBoing.h>

void HandMillObject::Awake()
{
	AddComponent<BoxCollider>();
	AddComponent<HandMillCooker>();
	AddComponent<GridPlacementSystem>();
	AddComponent<BoingBoing>();
}
