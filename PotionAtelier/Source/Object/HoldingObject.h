#pragma once
#include "framework.h"

class HoldingObject : public GameObject
{
	SERIALIZED_OBJECT(HoldingObject);
public:
	void Awake() override;
};