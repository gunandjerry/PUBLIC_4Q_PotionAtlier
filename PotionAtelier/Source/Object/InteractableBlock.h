#pragma once
#include "framework.h"

class InteractableBlock : public CubeObject
{
	SERIALIZED_OBJECT(InteractableBlock);

public:
	void Awake() override;

};