#pragma once
#include <Component/Base/Component.h>
#include <Component/Collider/Collider.h>

class BoxCollider : public Collider
{
	// width, height, depth
	Vector3 size{ 1, 1, 1 };

	bool debug_draw{ false };
	
	// test
	Vector3 prev_anchor{};
	Vector3 prev_size{};
public:
	BoxCollider() {}
	virtual ~BoxCollider();

private:
	void CreateShapeFromChildMesh();
public:

	void SetSize(float width, float height, float depth);
	virtual void SetSizeFollowingMesh() override; // 초기화용 / 메쉬 없는 애들은 그냥 기본 사이즈로 UpdateShape 호출해야 함.
	virtual void DrawBounds(Color color = { 1, 0, 0, 1 }) override;

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