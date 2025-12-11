#pragma once
#include <GameObject/Base/GameObject.h>

class SpawnerObject : public GameObject
{
	SERIALIZED_OBJECT(SpawnerObject)

public:
	virtual void Awake() override;

};

