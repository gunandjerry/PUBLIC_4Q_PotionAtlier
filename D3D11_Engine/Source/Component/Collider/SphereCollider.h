#pragma once
#include "Component/Base/Component.h"
#include "Component/Collider/Collider.h"

class SphereCollider : public Collider
{
	float radius{ 0.5f };
	bool debug_draw{ false };

	// test
	Vector3 prev_anchor{};
	float prev_radius{};
public:
	SphereCollider() {}
	virtual ~SphereCollider();

	float GetRadius() { return radius; }
	void SetSize(float radius);
	virtual void SetSizeFollowingMesh() override; // 초기화용 / 메쉬 없는 애들은 그냥 기본 사이즈로 UpdateShape 호출해야 함.
	virtual void DrawBounds(Color color = Color{ 1, 0, 0, 1 }) override;

	virtual void AttachShapeToActor() override;
	virtual void UpdateShape() override;
	virtual void DetachShapeToActor() override;

	virtual void SetIsTrigger(bool is_trigger) override;

	// test
	void UpdateAnchor();



public:
	virtual void Awake() override;
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override;
	virtual void LateUpdate() override {}


public:
	virtual void InspectorImguiDraw() override;

	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
};
