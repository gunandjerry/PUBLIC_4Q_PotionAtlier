#pragma once
#include "framework.h"

/*
class DummyComponent : public Component
{

public:
	virtual void Awake() override {}
protected:
	virtual void Start() override {}
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}

public:
	virtual void InspectorImguiDraw() override
	{
		ImGui::PushID(GetComponentIndex());
		if (ImGui::TreeNode("DummyComponent"))
		{
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	virtual void Serialized(std::ofstream& ofs) override
	{
		constexpr size_t header = (std::numeric_limits<size_t>::max)();
		constexpr uint32_t version = 0;
		Binary::Write::data(ofs, header); //헤더
		Binary::Write::data(ofs, version); //버전
	}
	virtual void Deserialized(std::ifstream& ifs) override
	{
		size_t header = Binary::Read::data<size_t>(ifs);
		uint32_t version = 0;
		if (header != (std::numeric_limits<size_t>::max)())
		{
		}
		else
		{
			version = Binary::Read::data<uint32_t>(ifs);
		}
	}
};
*/