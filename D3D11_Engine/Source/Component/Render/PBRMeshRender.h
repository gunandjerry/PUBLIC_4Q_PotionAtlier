#pragma once
#include <Component/Render/SimpleMeshRender.h>
#include <GameObject/Mesh/PBRMeshObject.h>

class PBRMeshRender : public SimpleMeshRender
{
public:
	PBRMeshRender();
	virtual ~PBRMeshRender() override = default;

	virtual void Awake() override;
	virtual void UpdateMeshDrawCommand() override;

	// test
	MeshDrawCommand* GetMeshDrawCommand() { return &meshDrawCommand; }

	PBRMeshObject& GetPBRMeshObject() { return *meshObject; }
private:
	PBRMeshObject* meshObject = nullptr;
};