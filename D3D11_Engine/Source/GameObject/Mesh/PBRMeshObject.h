#pragma once
#include <GameObject/Base/GameObject.h>

class PBRMeshObject : public GameObject
{
	SERIALIZED_OBJECT(PBRMeshObject)
public:
	PBRMeshObject() = default;
	virtual void Awake() override;

	virtual ~PBRMeshObject() override = default;
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);
};