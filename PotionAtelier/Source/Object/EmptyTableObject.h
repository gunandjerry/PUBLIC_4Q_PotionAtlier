#pragma once
#include "framework.h"

class EmptyTableObject : public GameObject
{
	SERIALIZED_OBJECT(EmptyTableObject);

public:
	void Awake() override;
};

class ColloderObject : public GameObject
{
	SERIALIZED_OBJECT(ColloderObject);

public:
	void Awake() override;
};