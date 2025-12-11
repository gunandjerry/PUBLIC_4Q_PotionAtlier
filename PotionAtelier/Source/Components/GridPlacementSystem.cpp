#include "GridPlacementSystem.h"
#include "Utility/SerializedUtility.h"

float GridPlacementSystem::tile_size = 1.0f;

void GridPlacementSystem::Update()
{
#ifdef _EDITOR
	if (Scene::EditorSetting.IsPlay()) return;
#endif
	if (activate == false) return;

	Vector3 cp = transform.position;
	Vector3 tp = { AdjustPosition(cp.x), AdjustPosition(cp.y), AdjustPosition(cp.z) };

	transform.SetPosition(tp);
}

void GridPlacementSystem::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("Grid Placement System"))
	{
		ImGui::Checkbox("Activate System", &activate);
		ImGui::DragFloat("Tile Size (Global)", &tile_size, 0.1f, 0.1f, 10.0f);
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GridPlacementSystem::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, tile_size);
	Binary::Write::data(ofs, activate);
}

void GridPlacementSystem::Deserialized(std::ifstream& ifs)
{
	tile_size = Binary::Read::data<float>(ifs);
	activate = Binary::Read::data<bool>(ifs);
}

float GridPlacementSystem::AdjustPosition(float cp)
{
	return std::round(cp / tile_size) * tile_size;
}
