#include "EmptyBoxColliderObject.h"

void EmptyBoxColliderObject::Awake()
{
	AddComponent<BoxCollider>();
}