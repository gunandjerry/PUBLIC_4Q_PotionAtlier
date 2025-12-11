#pragma once
#include <GameObject/Base/GameObject.h>

class GnomeCustomer : public GameObject
{
	SERIALIZED_OBJECT(GnomeCustomer)

public:
	virtual void Awake() override;

};