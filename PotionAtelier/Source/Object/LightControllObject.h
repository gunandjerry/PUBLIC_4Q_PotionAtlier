#pragma once

#include "framework.h"

class LightControll : public GameObject
{
	SERIALIZED_OBJECT(LightControll)
public:
	LightControll();
	virtual ~LightControll() override = default;
};

