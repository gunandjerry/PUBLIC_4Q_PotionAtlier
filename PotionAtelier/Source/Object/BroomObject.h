#pragma once
#include "framework.h"

class BroomObject : public GameObject
{
	SERIALIZED_OBJECT(BroomObject);


	//std::shared_ptr<class PhysicsMaterial> broom_material{ nullptr };
public:
	void Awake() override;
};
