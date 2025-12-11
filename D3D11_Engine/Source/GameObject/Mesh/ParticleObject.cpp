#include "ParticleObject.h"

ParticleSpawnerObject::ParticleSpawnerObject()
	: particleSpawnComponenet(AddComponent<ParticleSpawnComponent>())
{
}
