#include "FlyingGnome.h"
#include "Component/CharacterController/CharacterController.h"
#include "Core/TimeSystem.h"
#include "CustomerAI.h"

using namespace TimeSystem;

void FlyingGnome::Start()
{
	pressed_gnome_image = &AddComponent<UIRenderComponenet>();
	pressed_gnome_image->drawSpeed = 20;
}

void FlyingGnome::Update()
{
	if (let_me_fly == false) return;

	if (show_ui == false)
	{
		t += Time.DeltaTime * fly_speed;

		if (t < fly_limit_step_factor)
		{
			Vector3 new_pos = Vector3::Lerp(original_pos, goal_pos, t);
			transform.SetPosition(new_pos);

			//transform.SetRotation(Quaternion::Slerp(init_rotation, goal_rotation, t));
		}
		else if (t >= fly_limit_step_factor)
		{
			transform.SetPosition({ 0, -20000, 0 });
			show_ui = true;

			CustomerAI* customer_info = &GetComponent<CustomerAI>();
			if (customer_info->customer_type == Customer::Gnome)
			{
				pressed_gnome_image->SetTexture(L"./Resource/UI/Gnome/2d_GnomeHit.png");
			}
			else if (customer_info->customer_type == Customer::Mage)
			{
				pressed_gnome_image->SetTexture(L"./Resource/UI/Gnome/2d_MagicianHit.png");
			}


			SIZE clientSize = D3D11_GameApp::GetClientSize();
			ui_pos = { clientSize.cx * 0.5f, clientSize.cy * 0.5f };
			pressed_gnome_image->Enable = true;
			pressed_gnome_image->SetTransform(ui_pos.x, ui_pos.y, ui_size.x, ui_size.y);
			t = 0.0f;
			slip_step = 0;
		}
	}
	else if (show_ui == true)
	{
		t += Time.DeltaTime;

		if (slip_step == 0)
		{
			if (t >= ui_stop_duration)
			{
				++slip_step;
				t = 0;
			}
		}
		else if (slip_step == 1)
		{
			ui_pos -= Vector2{ 0, -ui_slip_speed_px_per_sec * Time.DeltaTime };
			pressed_gnome_image->SetPosition(ui_pos.x, ui_pos.y);
			if (t >= ui_slip_without_fade_out_duration)
			{
				++slip_step;
				t = 0;
			}
		}
		else if (slip_step == 2)
		{
			ui_pos -= Vector2{ 0, -ui_slip_speed_px_per_sec * Time.DeltaTime };
			pressed_gnome_image->SetPosition(ui_pos.x, ui_pos.y);
			pressed_gnome_image->SetColor({ 1, 1, 1, 1 - (t / ui_slip_with_fade_out_duration)});

			if (t >= ui_slip_with_fade_out_duration)
			{
				let_me_fly = false;
				pressed_gnome_image->SetColor({ 1, 1, 1, 0 });
				pressed_gnome_image->Enable = false;
				GetComponent<CustomerAI>().ResetFlag();
			}
		}
	}
}

void FlyingGnome::FlyMeToTheMoon()
{
	if (controller == nullptr) controller = gameObject.IsComponent<CharacterController>();
	if (controller != nullptr) controller->GRAVITY_MULTIPLIER = 0.0f;
	let_me_fly = true;

	CustomerAI* customer_info = &GetComponent<CustomerAI>();
	customer_info->punched = true;
	customer_info->SetAnimationClip(GnomeAnimType::Hit);

	Camera* cam = Camera::GetMainCamera();
	Vector3 cam_pos = cam->transform.position;
	Vector3 cam_forward = cam->transform.Forward;
	Vector3 cam_below = -cam->transform.Up;

	t = 0.0f;
	original_pos = transform.GetPosition();
	goal_pos = cam_pos;
	goal_pos += cam_forward * 20.0f;
	goal_pos += cam_below * 10.0f;

	/*goal_rotation = Quaternion::CreateFromYawPitchRoll(constant_rotation_euler.x, constant_rotation_euler.y, constant_rotation_euler.z);
	init_rotation = transform.GetRotation();*/
	transform.SetRotation(constant_rotation_euler);

	//Vector3 modified_original = original_pos + (transform.Up * 5.0f);
	//Vector3 rotation_axis = cam_pos - modified_original;
	//Vector3 local_x_rotation_axis = cam->transform.Right;
	//local_x_rotation_axis.Normalize();

	//Matrix objectRotation = Matrix::CreateLookAt(cam_pos, modified_original, Vector3::Up).Invert();
	//Quaternion rotationQuat = Quaternion::CreateFromRotationMatrix(objectRotation);
	////Quaternion rotationQuat2 = Quaternion::CreateFromAxisAngle(objectRotation);
	//Quaternion rotationQuat2 = Quaternion::CreateFromAxisAngle(local_x_rotation_axis, -35);

	//transform.SetRotation(rotationQuat * rotationQuat2);
}

void FlyingGnome::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("FlyingGnome"))
	{
		ImGui::SliderFloat("Fly Speed", &fly_speed, 0.0f, 30.0f);
		ImGui::SliderFloat("Rotation Speed", &rotate_speed, 0.0f, 30.0f);

		ImGui::Dummy(ImVec2{ 0, 10 });
		ImGui::SliderFloat2("Image Size", &ui_size.x, 100.0f, 1200.0f);
		ImGui::SliderFloat("Image Slip Speed", &ui_slip_speed_px_per_sec, 0.0f, 500.0f);

		if (ImGui::Button("Let Me Fly"))
		{
			FlyMeToTheMoon();
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void FlyingGnome::Serialized(std::ofstream& ofs)
{
	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 0;

	Binary::Write::data(ofs, header);
	Binary::Write::data(ofs, version);

	/*Binary::Write::Vector2(ofs, ui_size);
	Binary::Write::data<float>(ofs, fly_speed);
	Binary::Write::data<float>(ofs, ui_stop_start);
	Binary::Write::data<float>(ofs, ui_stop_end);
	Binary::Write::data<float>(ofs, ui_slip);
	Binary::Write::data<float>(ofs, rotate_speed);
	Binary::Write::data<float>(ofs, rotate_center_adjust);
	Binary::Write::data<float>(ofs, init_rotate_angle_y_adjust);
	Binary::Write::data<float>(ofs, ui_slip_speed_px_per_sec);
	Binary::Write::data<float>(ofs, fly_limit_step_factor);*/
}

void FlyingGnome::Deserialized(std::ifstream& ifs)
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

	/*ui_size = Binary::Read::Vector2(ifs);
	fly_speed = Binary::Read::data<float>(ifs);
	ui_stop_start = Binary::Read::data<float>(ifs);
	ui_stop_end = Binary::Read::data<float>(ifs);
	ui_slip = Binary::Read::data<float>(ifs);
	rotate_speed = Binary::Read::data<float>(ifs);
	rotate_center_adjust = Binary::Read::data<float>(ifs);
	init_rotate_angle_y_adjust = Binary::Read::data<float>(ifs);
	ui_slip_speed_px_per_sec = Binary::Read::data<float>(ifs);
	fly_limit_step_factor = Binary::Read::data<float>(ifs);*/
}
