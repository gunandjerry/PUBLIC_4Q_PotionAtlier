#include "CameraMoveHelper.h"
#include <Core/TimeSystem.h>
#include <Component/Camera/Camera.h>
#include <Math/Mathf.h>
#include <Core/GameInputSystem.h>
#include <Utility/ImguiHelper.h>

CameraMoveHelper::CameraMoveHelper()
{
	rotSpeed = 0.1f;
	moveSpeed = 50.0f;
}

void CameraMoveHelper::Awake()
{
	
}

void CameraMoveHelper::InspectorImguiDraw()
{
	ImGui::DragFloat("Move Speed", &moveSpeed, 1.f, 1.f, 1000.f);
	ImGui::DragFloat("Rotation Speed", &rotSpeed, 0.1f, 0.1f, 1.f);
}

void CameraMoveHelper::FixedUpdate()
{

}

void CameraMoveHelper::Update()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay())
		return;
#endif // _EDITOR

	if (Camera* mainCame = Camera::GetMainCamera())
	{
		if (&mainCame->gameObject == &gameObject)
		{
			UpdateMovemont();
		}
	}
}

void CameraMoveHelper::LateUpdate()
{

}

void CameraMoveHelper::OnInputProcess(InputManager::Input& Input)
{
#ifdef _EDITOR
	if (Scene::EditorSetting.IsPlay())
		return;

	if (Input.IsKey(MouseKeys::rightButton))
	{
		Vector3 forward = transform.Forward;
		Vector3 right = transform.Right;

		if (gameInputSystem.IsKey(KeyboardKeys::W))
		{
			inputVector += forward;
		}
		else if (gameInputSystem.IsKey(KeyboardKeys::S))
		{
			inputVector += -forward;
		}

		if (gameInputSystem.IsKey(KeyboardKeys::A))
		{
			inputVector += -right;
		}
		else if (gameInputSystem.IsKey(KeyboardKeys::D))
		{
			inputVector += right;
		}

		if (gameInputSystem.IsKey(KeyboardKeys::Q))
		{
			inputVector += -transform.Up;
		}
		else if (gameInputSystem.IsKey(KeyboardKeys::E))
		{
			inputVector += transform.Up;
		}

		int wheel = Input.GetRelativeMouseWheel();
		if (wheel != 0)
		{
			float delta = float(wheel / wheel) * 5.f;
			inputSpeed = wheel > 0 ? delta : -delta;
		}

		const Mouse::State& MouseState = Input.GetMouseState();
		if (MouseState.positionMode == Mouse::Mode::MODE_RELATIVE)
		{
			Vector2 delta = Vector2(float(MouseState.x), float(MouseState.y)) * rotSpeed * Mathf::Deg2Rad;
			AddPitch(delta.x);
			AddYaw(delta.y);
		}
		else
		{
			Input.SetMouseMode(Mouse::Mode::MODE_RELATIVE);
		}
	}
	else
	{
		Input.SetMouseMode(Mouse::Mode::MODE_ABSOLUTE);
	}
#endif // _EDITOR
}

void CameraMoveHelper::AddYaw(float value)
{	
	yawRotation = value;
}

void CameraMoveHelper::UpdateMovemont()
{
	inputVector.Normalize();
	if (inputVector.Length() > 0.0f)
	{
		transform.position += inputVector * moveSpeed * TimeSystem::Time.DeltaTime;
		inputVector = Vector3::Zero;
	}
	if (yawRotation)
	{
		transform.rotation = Quaternion::CreateFromAxisAngle(Vector3::UnitX, yawRotation) * transform.rotation;
		yawRotation = 0;
	}
	if (pitchRotation)
	{
		transform.rotation *= Quaternion::CreateFromAxisAngle(Vector3::UnitY, pitchRotation);
		pitchRotation = 0;
	}
	if (inputSpeed)
	{
		moveSpeed += inputSpeed;
		moveSpeed = std::clamp(moveSpeed, 5.f, 1000.f);
		inputSpeed = 0;
	}
	inputVector = Vector3::Zero;
}

void CameraMoveHelper::AddPitch(float value)
{
	pitchRotation = value;
}
