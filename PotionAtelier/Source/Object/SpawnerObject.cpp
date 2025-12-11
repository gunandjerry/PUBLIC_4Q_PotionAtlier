#include "SpawnerObject.h"
#include <Components/CustomerSpawner.h>

void SpawnerObject::Awake()
{
	AddComponent<CustomerSpawner>();
}
