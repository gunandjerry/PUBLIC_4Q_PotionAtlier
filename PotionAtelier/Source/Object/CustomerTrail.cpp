#include "CustomerTrail.h"
#include <Components/WaypointComponent.h>

void CustomerTrail::Awake()
{
	AddComponent<WaypointComponent>();
}
