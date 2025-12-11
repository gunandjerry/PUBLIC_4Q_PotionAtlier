#include "CounterObject.h"
#include "../Components/DeliverCounter.h"
#include "../Components/GridPlacementSystem.h"
#include "../Components/BoingBoing.h"

void CounterObject::Awake()
{
	AddComponent<BoxCollider>();
	AddComponent<DeliverCounter>();
	AddComponent<GridPlacementSystem>();
	AddComponent<BoingBoing>();
}
