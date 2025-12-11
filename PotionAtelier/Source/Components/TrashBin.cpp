#include "TrashBin.h"
#include "PlayerController.h"
#include "Holding.h"

using namespace TimeSystem;

TrashBin::TrashBin()
{
	type = InteractableType::TrashBin;
}

void TrashBin::Start()
{
	FindHoldingObject(&transform);
}

void TrashBin::Update()
{
	if (on_throw == true && obj_mesh != nullptr)
	{
		t += Time.DeltaTime * shrink_speed;

		if (t >= 1.0f)
		{
			obj_mesh->SetEmpty();
			on_throw = false;
		}
		else
		{
			obj_mesh->transform.SetLocalPosition(Vector3::Lerp(obj_ori_position, goal_pos, t));
			obj_mesh->transform.SetLocalScale(Vector3::Lerp(obj_ori_scale, goal_scale, t));
			//cur_rotation.y += Time.DeltaTime * rotation_speed;
			//obj_mesh->transform.SetLocalRotation(cur_rotation);
		}
	}
}

void TrashBin::OnFocusIn(PlayerController* controller)
{
	//transform.SetScale({ 1.1f, 1.1f, 1.1f });
}

void TrashBin::OnFocusOut(PlayerController* controller)
{
	//transform.SetScale({ 1.0f, 1.0f, 1.0f });
}

bool TrashBin::OnInteract(PlayerController* controller)
{
	if (controller->something_on_hand == false) return false;

	if (obj_mesh == nullptr) FindHoldingObject(&transform);
	if (obj_mesh != nullptr)
	{
		obj_mesh->SetType(controller->hold_type, controller->hold_subtype);
		obj_mesh->transform.SetLocalPosition(obj_ori_position);
		obj_mesh->transform.SetLocalScale(obj_ori_scale);
		//cur_rotation = { 0, 0, 0 };
		//obj_mesh->transform.SetLocalRotation(cur_rotation);

		t = 0.0f;
		on_throw = true;
	}

	controller->PutDown();
	// 대충 쓰레기통에 버려지는 효과

	return true;
}

void TrashBin::FindHoldingObject(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"HoldingObject"))
		{
			obj_mesh = c->gameObject.IsComponent<Holding>();
			if (obj_mesh != nullptr)
			{
				obj_ori_position = obj_mesh->transform.GetLocalPosition();
				obj_ori_scale = obj_mesh->transform.GetLocalScale();

				goal_pos = obj_ori_position;
				goal_pos.y -= 1.0f;
				goal_pos.z += 3.7f; // 앵커가 바닥이라
				goal_scale = { 0, 0, 0 };

				return;
			}
		}
		FindHoldingObject(c);
	}

	return;
}
