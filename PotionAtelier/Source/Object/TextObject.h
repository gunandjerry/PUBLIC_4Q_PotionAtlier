#pragma once
#include "framework.h"

class TextObject : public GameObject
{
	SERIALIZED_OBJECT(TextObject);
public:
	void Awake() override;
};
