#include "BroomComponent.h"
#include "PlayerController.h"

void BroomComponent::Start()
{
	//init_position = transform.GetPosition();
	//init_rotation = { 0, 0, 0 };
	//init_scale = transform.GetScale();
}

void BroomComponent::Update()
{
	
}

//void BroomComponent::OnFocusIn(PlayerController* controller)
//{
//
//}
//
//void BroomComponent::OnFocusOut(PlayerController* controller)
//{
//
//}
//
//bool BroomComponent::OnInteract(PlayerController* controller)
//{
//	if (on_hold == true)
//	{
//		PutDown(controller);
//		return true;
//	}
//
//	if (controller->something_on_hand == true) return false;
//
//	Hold(controller);
//	return true;
//}
//
//void BroomComponent::PutDown(PlayerController* controller)
//{
//	
//}
//
//void BroomComponent::Hold(PlayerController* controller)
//{
//}

//void BroomComponent::LockRotation()
//{
//	PhysicsActor* actor = gameObject.GetPhysicsActor();
//	if (actor != nullptr)
//	{
//		actor->SetFreezeRotateX(true);
//		actor->SetFreezeRotateY(true);
//		actor->SetFreezeRotateZ(true);
//	}
//}
//
//void BroomComponent::UnlockRotation()
//{
//	PhysicsActor* actor = gameObject.GetPhysicsActor();
//	if (actor != nullptr)
//	{
//		actor->SetFreezeRotateX(false);
//		actor->SetFreezeRotateY(false);
//		actor->SetFreezeRotateZ(false);
//	}
//}
