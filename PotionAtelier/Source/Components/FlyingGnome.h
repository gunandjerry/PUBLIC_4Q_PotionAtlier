#pragma once
#include <framework.h>
#include <Component/Base/Component.h>


class FlyingGnome : public Component
{
public:
	virtual void Awake() {}
	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}


private:
	class CharacterController* controller{ nullptr };
	bool show_ui = false;
	Vector2 ui_pos{ 0,0 };
	bool let_me_fly{ false };
	float t{ 0.0f };
	Vector3 original_pos{ 0,0,0 };
	Vector3 goal_pos{ 0,0,0 };
	Vector3 rotation{ 0, 0, 0 };
	Vector3 to_cam_axis{ 0, 0, 0 };
	Vector3 center_anchor{ 0,0,0 };
	UIRenderComponenet* pressed_gnome_image{ nullptr };


	// Serializable
	float fly_limit_step_factor{ 1.0f };
	Vector2 ui_size{ 1920, 1080 };
	float fly_speed = 0.5f;
	// step factor
	int slip_step{ 0 };
	float ui_stop_duration{ 1.2f };
	float ui_slip_without_fade_out_duration{ 0.6f };
	float ui_slip_with_fade_out_duration{ 0.6f };

	float rotate_speed{ 0.0f };
	float rotate_center_adjust{ 8.0f };
	float init_rotate_angle_y_adjust{ 12.5f };

	float ui_slip_speed_px_per_sec{ 100.0f };



	/*Quaternion init_rotation{};
	Quaternion goal_rotation{};*/
	Vector3 constant_rotation_euler{ -27.072, 177.008, 7.008 };

public:
	void FlyMeToTheMoon();


	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;


};

