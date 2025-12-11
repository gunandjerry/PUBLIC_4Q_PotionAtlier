#pragma once
#include "framework.h"

class Obstacle : public CubeObject
{
	SERIALIZED_OBJECT(Obstacle);

public:
	void Awake() override;
};