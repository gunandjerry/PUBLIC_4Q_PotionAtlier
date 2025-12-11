#pragma once
#include <GameObject/Base/GameObject.h>

class CustomerTrail : public GameObject
{
	SERIALIZED_OBJECT(CustomerTrail);

public:
	void Awake() override;

};
