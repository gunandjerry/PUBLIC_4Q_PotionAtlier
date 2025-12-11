#pragma once
#include "framework.h"

class Map : public GameObject
{
	SERIALIZED_OBJECT(Map);

public:
	void Awake() override;
};