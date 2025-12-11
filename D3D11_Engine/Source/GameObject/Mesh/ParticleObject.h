#pragma once
#include <GameObject/Base/GameObject.h>
#include <Component\Render\ParticleDrawComponent.h>


class ParticleSpawnerObject : public GameObject
{
	SERIALIZED_OBJECT(ParticleSpawnerObject)
public:
	ParticleSpawnerObject();
	virtual ~ParticleSpawnerObject() override = default;


	ParticleSpawnComponent& particleSpawnComponenet;
private:
	using GameObject::transform;
};


