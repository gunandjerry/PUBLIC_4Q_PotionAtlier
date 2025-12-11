#include "InteractableBlock.h"
#include "../Components/Throwable.h"
#include "../Components/CuttingBoardCooker.h"

void InteractableBlock::Awake()
{
	PBRMeshObject::Awake();
	AddComponent<CubeMeshRender>();

	AddComponent<BoxCollider>();
	AddComponent<CuttingBoardCooker>();
}