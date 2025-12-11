#include "CauldronObject.h"
#include "../Components/Cauldron.h"
#include "../Components/GridPlacementSystem.h"
#include "../Components/BoingBoing.h"

void CauldronObject::Awake()
{
	AddComponent<BoxCollider>();
	AddComponent<Cauldron>();
	AddComponent<GridPlacementSystem>();
	AddComponent<BoingBoing>();
}
