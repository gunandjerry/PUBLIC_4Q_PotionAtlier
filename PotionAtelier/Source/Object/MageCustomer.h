#pragma once
#include <GameObject/Base/GameObject.h>

class MageCustomer : public GameObject
{
	SERIALIZED_OBJECT(MageCustomer)

public:
	virtual void Awake() override;

};