#include "IngredientStandObject.h"
#include "../Components/IngredientStand.h"
#include "../Components/GridPlacementSystem.h"
#include "../Components/BoingBoing.h"

void IngredientStandObject::Awake()
{
	AddComponent<BoxCollider>();
	AddComponent<IngredientStand>();
	AddComponent<GridPlacementSystem>();
	AddComponent<BoingBoing>();
}
