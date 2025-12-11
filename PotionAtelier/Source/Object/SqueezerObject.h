#pragma once
#include <GameObject/Base/GameObject.h>

class SqueezerObject : public GameObject
{
	SERIALIZED_OBJECT(SqueezerObject);

public:
	void Awake() override;

};
