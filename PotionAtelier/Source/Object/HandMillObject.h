#pragma once
#include <GameObject/Base/GameObject.h>

class HandMillObject : public GameObject
{
	SERIALIZED_OBJECT(HandMillObject);

public:
	void Awake() override;

};
