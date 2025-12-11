#include "Obstacle.h"

void Obstacle::Awake()
{
	PBRMeshObject::Awake();
	AddComponent<CubeMeshRender>();

	AddComponent<BoxCollider>();
}