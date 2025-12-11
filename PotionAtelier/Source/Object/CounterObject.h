#pragma once
#include "framework.h"

class CounterObject : public GameObject
{
	SERIALIZED_OBJECT(CounterObject);

public:
	void Awake() override;

};

