#pragma once
#include <map>
#include <string>
#include "framework.h"
#include "../HoldableTypes.h"

// Memo
// 이건 그냥 들고 있는 메쉬를 바꾸는 용도로만 쓰고
// 들고 있는 종류 등의 처리는 그냥 SamplerCharacterController에서 처리하게

class Holding : public Component
{
	GameObject* current_mesh{ nullptr };

public:
	virtual void Awake();
	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}



public:
	bool SetType(HoldableType type, UINT sub_type);
	void SetEmpty();
	void SetFocus(bool isFocus);

	virtual void InspectorImguiDraw() override;
	//virtual void Serialized(std::ofstream& ofs) override;
	//virtual void Deserialized(std::ifstream& ifs) override;
private:
	std::wstring GetObjectTag(HoldableType type, UINT sub_type);
};

