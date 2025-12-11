#pragma once
#include <vector>
#include <Component/Base/Component.h>
#include <Component/Collider/Collider.h>

class MeshCollider : public Collider
{
	std::vector<px::PxVec3> pxVertices;
	std::vector<UINT> indices;

	//bool debug_draw{ false };
public:
	MeshCollider() {}
	virtual ~MeshCollider();



	void CreateShapeFromChildMesh();
private:
	void Bake();
	void BakeOnStart();
public:

	virtual void SetSizeFollowingMesh() override {}
	virtual void DrawBounds(Color color = { 1, 0, 0, 1 }) override {}

	virtual void AttachShapeToActor() override;
	virtual void UpdateShape() override {}
	virtual void DetachShapeToActor() override;

	virtual void SetIsTrigger(bool is_trigger) override;



public:
	virtual void Awake() override;
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}



public:
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Serialized(std::ofstream& ofs) override;
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Deserialized(std::ifstream& ifs) override;
	virtual void InspectorImguiDraw() override;
};