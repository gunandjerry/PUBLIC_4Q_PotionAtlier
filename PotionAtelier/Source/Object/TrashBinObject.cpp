#include "TrashBinObject.h"
#include "../Components/TrashBin.h"
#include "../Components/GridPlacementSystem.h"
#include "../Components/BoingBoing.h"

void TrashBinObject::Awake()
{
	AddComponent<BoxCollider>();
	AddComponent<TrashBin>();
	AddComponent<GridPlacementSystem>();
	AddComponent<BoingBoing>();
}
