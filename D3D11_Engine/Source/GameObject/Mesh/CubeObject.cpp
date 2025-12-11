#include "CubeObject.h"
#include <framework.h>
#include <Utility/SerializedUtility.h>

void CubeObject::Awake()
{
	PBRMeshObject::Awake();
	AddComponent<CubeMeshRender>();
}

