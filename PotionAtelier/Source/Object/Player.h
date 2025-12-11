#pragma once
#include "framework.h"

class Player : public GameObject
{
	SERIALIZED_OBJECT(Player);

public:
	void Awake() override;

};