#include "EmptyTableObject.h"
#include "../Components/EmptyTable.h"
#include "../Components/GridPlacementSystem.h"
#include "../Components/BoingBoing.h"

void EmptyTableObject::Awake()
{
	AddComponent<BoxCollider>();
	AddComponent<GridPlacementSystem>();
	AddComponent<EmptyTable>();
	AddComponent<BoingBoing>();
}

void ColloderObject::Awake()
{
	AddComponent<BoxCollider>();
}
