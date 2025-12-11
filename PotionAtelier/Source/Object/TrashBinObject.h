#pragma once
#include "framework.h"

class TrashBinObject : public GameObject
{
	SERIALIZED_OBJECT(TrashBinObject);

public:
	void Awake() override;

};

