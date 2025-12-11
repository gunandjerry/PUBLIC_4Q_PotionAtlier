#include "BroomObject.h"
#include "../Components/BroomComponent.h"
#include "../Components/GridPlacementSystem.h"
#include "../Components/BoingBoing.h"

void BroomObject::Awake()
{
	//BoxCollider* col1 = &AddComponent<BoxCollider>();
	//BoxCollider* col2 = &AddComponent<BoxCollider>();

	//UINT player_box_collider_slot = 10;
	//UINT broom_collider_slot = 11;

	//PhysicsManager::GetInstance().SetPhysicsLayer(col1, broom_collider_slot);
	//PhysicsManager::GetInstance().SetPhysicsLayer(col2, broom_collider_slot);


	/*broom_material = std::make_shared<PhysicsMaterial>(PhysicsManager::GetInstance().GetPhysics());
	broom_material->SetStaticFriction(10.0f);
	broom_material->SetFrictionCombineMode(PhysicsMaterialCombineMode::Max);

	col1->SetPhysicsMateiral(broom_material.get());
	col2->SetPhysicsMateiral(broom_material.get());
	GetPhysicsActor()->SetMass(100);*/

	//AddComponent<Rigidbody>();
	AddComponent<BroomComponent>();
	//AddComponent<GridPlacementSystem>();
	AddComponent<BoingBoing>();
}