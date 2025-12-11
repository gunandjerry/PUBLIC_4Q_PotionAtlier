#pragma once
#include "framework.h"

class CauldronObject : public GameObject
{
	SERIALIZED_OBJECT(CauldronObject);

public:
	void Awake() override;
};

