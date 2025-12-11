#pragma once
#include <framework.h>


class GridPlacementSystem : public Component
{
	bool activate{ false };
	static float tile_size;

public:
	virtual void Awake() {}
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}
public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;



private:
	float AdjustPosition(float curPos);
};

