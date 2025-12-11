#include "Map.h"

void Map::Awake()
{
	MeshCollider* collider = &AddComponent<MeshCollider>();
	



	// 빗자루 밖으로 빠져나가지 않게 하는 용도
	//AddComponent<BoxCollider>();
	//AddComponent<BoxCollider>();
	//AddComponent<BoxCollider>();
	//AddComponent<BoxCollider>();
	//AddComponent<BoxCollider>();
}
