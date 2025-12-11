#pragma once
#include <vector>
#include <Math/Mathf.h>
#include <Component/Base/Component.h>

struct WaypointData
{
    Vector3 position;
    bool is_stop_point = false;
};

class WaypointComponent : public Component
{
public:
    virtual void Awake() override {}

    virtual void Serialized(std::ofstream& ofs) override;
    virtual void Deserialized(std::ifstream& ifs) override;
    virtual void InspectorImguiDraw() override;

protected:
    virtual void FixedUpdate() override {}
    virtual void Update() override;
    virtual void LateUpdate() override {}

public:
    const std::vector<WaypointData>& GetWaypoints() const { return waypoints; }

private:
    std::vector<WaypointData> waypoints;
    bool stoppoint_set = false;

};