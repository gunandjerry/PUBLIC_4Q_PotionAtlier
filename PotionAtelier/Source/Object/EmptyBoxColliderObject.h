#pragma once
#include "framework.h"

class EmptyBoxColliderObject : public GameObject
{
	SERIALIZED_OBJECT(EmptyBoxColliderObject);

public:
	void Awake() override;
};