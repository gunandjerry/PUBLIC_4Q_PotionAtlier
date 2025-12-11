#include "CuttingBoardObject.h"
#include "../Components/Holding.h"
#include "../Components/CuttingBoardCooker.h"
#include "../Components/GridPlacementSystem.h"
#include "../Components/BoingBoing.h"

void CuttingBoardObject::Awake()
{
	AddComponent<BoxCollider>();
	AddComponent<CuttingBoardCooker>();
	AddComponent<GridPlacementSystem>();
	AddComponent<BoingBoing>();
}
