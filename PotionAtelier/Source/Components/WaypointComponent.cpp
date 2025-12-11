#include "WaypointComponent.h"
#include <framework.h>
#include <ImGuizmo/ImGuizmo.h>

void WaypointComponent::Update()
{
#ifdef _EDITOR
    if (Scene::EditorSetting.IsPlay())
        return;

    if (waypoints.size() < 2)
        return;

    for (size_t i = 0; i < waypoints.size() - 1; ++i)
    {
        const Matrix& projection = Camera::GetMainCamera()->GetPM();
        const Vector3& start = waypoints[i].position;
        const Vector3& end = waypoints[i + 1].position;

        DebugMeshDrawCommand command;
        command.type = EDebugMeshDraw::Type::Ray;
        command.ray.position = Vector3::Transform(start, projection);
        command.ray.direction = Vector3::Transform(end - start, projection);
        command.color = Color{ 0.0f, 1.0f, 0.0f, 0.0f };
        D3D11_GameApp::GetRenderer().AddDrawCommand(command);

        if (waypoints[i].is_stop_point == true)
        {
            DebugMeshDrawCommand command;
            command.type = EDebugMeshDraw::Type::Sphere;
            command.boundingSphere.Center = Vector3::Transform(start, projection);
            command.boundingSphere.Radius = 5.f;
            command.color = Color{ 1.0f, 0.0f, 0.0f, 0.0f };
            D3D11_GameApp::GetRenderer().AddDrawCommand(command);
        }
    }
#endif
}

void WaypointComponent::Serialized(std::ofstream& ofs)
{
    using namespace Binary;

    Write::data(ofs, waypoints.size());
    for (auto& waypoint : waypoints)
    {
        Write::Vector3(ofs, waypoint.position);
        Write::data<bool>(ofs, waypoint.is_stop_point);
    }
}

void WaypointComponent::Deserialized(std::ifstream& ifs)
{
    using namespace Binary;

    size_t size = Read::data<size_t>(ifs);
    waypoints.resize(size);
    for (size_t i = 0; i < size; ++i)
    {
        waypoints[i].position = Read::Vector3(ifs);
        waypoints[i].is_stop_point = Read::data<bool>(ifs);
    }
}

void WaypointComponent::InspectorImguiDraw()
{
    ImGui::Text("Waypoints");


    if (ImGui::Button("Add Waypoint"))
    {
        Vector3 pos = gameObject.transform.position;
        waypoints.emplace_back(WaypointData{ pos, false });
    }

    if (stoppoint_set == false)
    {
        if (ImGui::Button("Add Stoppoint"))
        {
            Vector3 pos = gameObject.transform.position;
            waypoints.emplace_back(WaypointData{ pos, true });
            stoppoint_set = true;
        }
    }
    if (ImGui::Button("Remove Last Waypoints") && !waypoints.empty())
    {
		if (waypoints.back().is_stop_point)
			stoppoint_set = false;

        waypoints.pop_back();
    }

    if (ImGui::Button("Clear All Waypoints"))
    {
        waypoints.clear();
        stoppoint_set = false;
    }

    ImGui::Separator();
}
