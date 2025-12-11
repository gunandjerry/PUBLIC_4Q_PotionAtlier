#include "CustomerAI.h"
#include <framework.h>
#include <Component/CharacterController/CharacterController.h>
#include <Component/Animation/TransformAnimation.h>
#include <Object/GameManager.h>
#include <Components/GameManagerComponent.h>
#include <Components/CustomerSpawner.h>
#include <Asset/MaterialAsset.h>
#include <Components/TextBubble.h>
#include <StringResource.h>

static PotionType GetRandomPotion(const std::vector<std::pair<PotionType, int>>& weightedPotions)
{
	int totalWeight = 0;
	for (const auto& pair : weightedPotions)
		totalWeight += pair.second;

	int randValue = Random::Range(1, totalWeight);
	int cumulativeWeight = 0;

	for (const auto& pair : weightedPotions)
	{
		cumulativeWeight += pair.second;
		if (randValue <= cumulativeWeight)
			return pair.first;
	}

	return PotionType::HealthPotion; // 기본값
}

void CustomerAI::AssignPotionBasedOnStage()
{
	wanted_potion = GetRandomPotion(GameManager::GetGM().GetCurrentPotionWeight());
}

CustomerAI::CustomerAI()
{

}

CustomerAI::~CustomerAI()
{
	for (auto& state : state_map)
	{
		delete state.second;
	}
}

void CustomerAI::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("CharacterController"))
	{
		float height = controller->GetHeight();
		float radius = controller->GetRadius();
		ImGui::Text("Collider Size : %f, %f", height, radius);
		ImGui::SliderFloat("Max Movement Speed", &max_speed, 0.0f, 100.0f);

		if (ImGui::Button("Set Collider Size Automatically"))
			controller->SetSizeAutomatically();

		static float _temp[2]{ 0.0f, 0.0f };
		ImGui::InputFloat2("Set Size:", &_temp[0]);
		if (ImGui::Button("Set Collider Size Manually"))
		{
			if (_temp[0] <= 0.0f) _temp[0] = 0.1f;
			if (_temp[1] <= 0.0f) _temp[1] = 0.1f;
			controller->SetSize(_temp[0], _temp[1]);
		}
		ImGui::TreePop();
	}
	ImGui::InputText("VFX_Hit", (char*)hit.c_str(), hit.size(), ImGuiInputTextFlags_CallbackResize,
		[](ImGuiInputTextCallbackData* data)
		{
			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
			{
				std::string* str = (std::string*)data->UserData;
				IM_ASSERT(data->Buf == str->c_str());
				str->resize(data->BufTextLen);
				data->Buf = str->data();
			}
			return 0;
		}, & hit);
	if (ImGui::Button("Save"))
	{
		SaveVFX();
	}


	if (ImGui::Button((const char*)u8"주문하기"))
	{
		ChangeState(CustomerStateType::Order);
	}

	ImGui::PopID();
}

void CustomerAI::Awake()
{
	state_map[CustomerStateType::Run] = new RunState();
	state_map[CustomerStateType::Wait] = new WaitState();
	state_map[CustomerStateType::Order] = new OrderState();
	state_map[CustomerStateType::Hit] = new HitState();
	state_map[CustomerStateType::Fever] = new FeverState();
	state_map[CustomerStateType::Exit] = new ExitState();

	ChangeState(CustomerStateType::Run);

	gameObject.SetTag(L"Customer");
	if (GameObject* trail = sceneManager.FindObject(L"CustomerTrail"))
	{
		waypoints = trail->GetComponent<WaypointComponent>().GetWaypoints();
		waypoints_count = static_cast<int>(waypoints.size());
	}
	                                                   
	std::wstring wstr = std::wstring(hit.begin(), hit.end());
	if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
	{
		VFX_Hit = findItem->IsComponent<ParticleSpawnComponent>();
	}

	int stage = GameManager::GetGM().GetCurrentStageNum();
	if (stage == 0)
	{
		wanted_potion = PotionType::HealthPotion;
	}
	else if (stage == 1)
	{
		int rand = Random::Range(1, 2);
		switch (rand)
		{
		case 1:
			wanted_potion = PotionType::HealthPotion;
		break;
		case 2:
			wanted_potion = PotionType::ManaPotion;
			break;
		default:
			wanted_potion = PotionType::HealthPotion;
		break;
		}
	}
	else if (stage == 2)
	{
		int rand = Random::Range(1, 4);
		switch (rand)
		{
		case 1:
			wanted_potion = PotionType::HealthPotion;
		break;
		case 2:
			wanted_potion = PotionType::ManaPotion;
		break;
		case 3:
			wanted_potion = PotionType::SeductionPotion;
		break;
		case 4:
			wanted_potion = PotionType::AddictionPotion;
		break;
		default:
			wanted_potion = PotionType::HealthPotion;
		break;
		}
	}
	else if (stage == 3)
	{
		int rand = Random::Range(1, 6);
		wanted_potion = static_cast<PotionType>(1 << rand);
	}

	//Success = [this]()
	//	{
	//		GameManagerComponent& gm = GameManager::GetGM();
	//		ChangeState(CustomerStateType::Run);
	//		float t = static_cast<OrderState*>(state_map[CustomerStateType::Order])->t;
	//		int tip;
	//		const auto& timeData = CustomerSpawner::GetTimeData(gm.GetCurrentStageNum());
	//		if (t < timeData.AngerTime) //분노
	//			tip = 0;
	//		if (t < timeData.NervousTime)//초조
	//			tip = 10;
	//		else       
	//			tip = 25;
	//		GameManager::GetGM().AddScore(wanted_potion, tip);
	//		isOrderEnd = true;



	//		auto& tb = GetComponent<TextBubble>();
	//		tb.ShowBubble();
	//		int i = Random::Range(10, 13);
	//		tb.SetBubbleText(StringResource::GetTutorialText(std::format(L"CT_{}", i)));

	//		TimeSystem::Time.DelayedInvok([&tb]()
	//		{
	//			tb.HideBubble();
	//		}, TextBubble::text_bubble_duration_common);
	//	};

	//Fail = [this]()
	//	{
	//		isOrderEnd = true;
	//		ChangeState(CustomerStateType::Run);



	//		auto& tb = GetComponent<TextBubble>();
	//		tb.ShowBubble();
	//		int i = Random::Range(14, 15);
	//		tb.SetBubbleText(StringResource::GetTutorialText(std::format(L"CT_{}", i)));

	//		TimeSystem::Time.DelayedInvok([&tb]()
	//		{
	//			tb.HideBubble();
	//		}, TextBubble::text_bubble_duration_common);
	//	};

	LoadVFX();
}

void CustomerAI::Start()
{
	const auto& timeData = CustomerSpawner::GetTimeData(GameManager::GetGM().GetCurrentStageNum());
	max_patience = timeData.AngerTime + timeData.NervousTime + timeData.CalmTime;

	if (GameObject* trail = sceneManager.FindObject(L"CustomerTrail"))
	{
		waypoints = trail->GetComponent<WaypointComponent>().GetWaypoints();
		waypoints_count = static_cast<int>(waypoints.size());
	}

	if (customer_animator == nullptr)
		FindAnimationObject(&transform);

	if (customer_animator)
		customer_animator->PlayClip(ResourceFinder::GetGnomeAnimationClipName(GnomeAnimType::Run));

	if (Transform* child = transform.GetChild(0))
	{
		if (Transform* child1 = child->GetChild(0))
		{
			if (PBRBoneMeshRender* outpit = child1->gameObject.GetComponentAtIndex<PBRBoneMeshRender>(1))
			{
				outpit->materialAsset.customData.SetField("hue", Random::Range(0.f, 1.f));
			}
		}
	}
}

void CustomerAI::Update()
{
	if (controller == nullptr)
	{
		controller = &GetComponent<CharacterController>(); 
	}

#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay())
		return;
#endif

	if (punched)
	{
		if (VFX_Hit) VFX_Hit->CreateParticle();
		GetComponent<TextBubble>().HideBubble();
		return;
	}

	if (current_waypoint_index >= waypoints_count - 1)
	{
		ResetFlag();
		return;		
	}


	if (current_state)
	{
		current_state->Update(this);
	}
}

void CustomerAI::ChangeState(CustomerStateType new_state)
{
	prev_state_type = curr_state_type;
	curr_state_type = new_state;

	current_state = state_map[new_state];
	
	if (current_state)
		current_state->Enter(this);
}

void CustomerAI::ChangePrevState()
{
	ChangeState(prev_state_type);
}

void CustomerAI::ResetFlag()
{
	CustomerSpawner::GnomeCounter--;
	gameObject.Active = false;
	this->isOrderEnd = false;
	this->punched = false;
	this->myOrderQueueIndex = -1;
	this->current_waypoint_index = 0;
	ChangeState(CustomerStateType::Run);
}

bool CustomerAI::RayForward()
{
	RaycastResult result = Physics::Sweep(SweepShape::Sphere, ray_size.x, ray_size.y, transform.GetPosition(), transform.Forward, ray_distance, 1, &GetGameObject());
	if (!result.hits.empty())
	{
		if (auto& tags = result.hits[0].object->GetTags(); tags.find(L"Customer") != tags.end())
		{
			controller->Move(Vector3::Zero);

			return true;
		}
	}

	return false;
}

void CustomerAI::Move()
{
	if (waypoints.empty()) return;
	if (current_waypoint_index >= waypoints_count - 1) return;

	float dt = TimeSystem::Time.DeltaTime;

	current_speed = std::lerp(current_speed, max_speed, dt);
	current_speed = std::max(current_speed, 0.1f);

	Vector3 next_position;
	if (waypoints[current_waypoint_index + 1].is_stop_point)
	{
		float t = (float)GameManager::GetGM().GetOrderCount() / 5.f;
		next_position = Vector3::Lerp(waypoints[current_waypoint_index + 1].position, waypoints[current_waypoint_index].position, t);
	}
	else
		next_position = waypoints[current_waypoint_index + 1].position;

	Vector3 prev_position = transform.position;
	Vector3 movement = next_position - prev_position;
	movement.y = 0.0f;
	float distance = movement.Length();
	if (distance > 0.0f)
	{
		movement.Normalize();
	}

	movement *= current_speed;

	if (distance > 0.0f)
	{
		float theta = std::atan2f(movement.x, movement.z);
		Matrix rotation_matrix = Matrix::CreateFromYawPitchRoll(0, theta, 0);
		separated_interact_direction = Vector3::Transform(Vector3::UnitZ, rotation_matrix);

		float pi2 = 2 * Mathf::PI;

		superficial_rotation = std::fmod(superficial_rotation, pi2);
		if (superficial_rotation < 0)
			superficial_rotation += pi2;

		float delta_angle = std::fmod(theta - superficial_rotation + pi2, pi2);
		if (std::abs(delta_angle) > Mathf::PI)
			delta_angle -= pi2 * (delta_angle > 0 ? 1 : -1);

		float rotate_amount = rotate_speed * dt;

		if (std::fabs(delta_angle) < rotate_amount)
		{
			superficial_rotation = theta;
		}
		else
		{
			superficial_rotation += (delta_angle > 0 ? 1 : -1) * rotate_amount;
		}

		superficial_rotation = std::fmod(superficial_rotation, pi2);
		if (superficial_rotation < 0)
			superficial_rotation += pi2;

		transform.rotation = { 0, superficial_rotation * Mathf::Rad2Deg, 0 };
		transform.position += movement * dt;
		controller->Move(movement); 
		
	}

	if (distance < 4.f)
	{
		current_waypoint_index++;
	}
}

void CustomerAI::MoveSort()
{
	if (isSortEnd)
		return;

	float dt = TimeSystem::Time.DeltaTime;

	current_speed = std::lerp(current_speed, max_speed, dt);
	current_speed = std::max(current_speed, 0.1f);

	float t = (float)myOrderQueueIndex / 5.f;
	Vector3 next_position = Vector3::Lerp(waypoints[current_waypoint_index].position, waypoints[current_waypoint_index - 1].position, t);
	Vector3 prev_position = transform.position;
	Vector3 movement = next_position - prev_position;
	movement.y = 0.0f;
	float distance = movement.Length();
	if (distance >= 4.0f)
	{
		movement.Normalize();
	}
	if (distance < 4.f)
	{
		SetAnimationClip(GnomeAnimType::Idle);
		isSortEnd = true;
		return;
	}

	movement *= current_speed;
	if (distance > 0.0f)
	{
		float theta = std::atan2f(movement.x, movement.z);
		Matrix rotation_matrix = Matrix::CreateFromYawPitchRoll(0, theta, 0);
		separated_interact_direction = Vector3::Transform(Vector3::UnitZ, rotation_matrix);

		float pi2 = 2 * Mathf::PI;

		superficial_rotation = std::fmod(superficial_rotation, pi2);
		if (superficial_rotation < 0)
			superficial_rotation += pi2;

		float delta_angle = std::fmod(theta - superficial_rotation + pi2, pi2);
		if (std::abs(delta_angle) > Mathf::PI)
			delta_angle -= pi2 * (delta_angle > 0 ? 1 : -1);

		float rotate_amount = rotate_speed * dt;

		if (std::fabs(delta_angle) < rotate_amount)
		{
			superficial_rotation = theta;
		}
		else
		{
			superficial_rotation += (delta_angle > 0 ? 1 : -1) * rotate_amount;
		}

		superficial_rotation = std::fmod(superficial_rotation, pi2);
		if (superficial_rotation < 0)
			superficial_rotation += pi2;

		transform.rotation = { 0, superficial_rotation * Mathf::Rad2Deg, 0 };
		transform.position += movement * dt;
		controller->Move(movement);
	}
}

void CustomerAI::FindAnimationObject(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (TransformAnimation* animator = c->gameObject.IsComponent<TransformAnimation>(); animator != nullptr)
		{
			customer_animator = animator;
		}

		if (customer_animator != nullptr) return;
		else
		{
			FindAnimationObject(c);
		}
	}
}

void CustomerAI::SetAnimationClip(GnomeAnimType type)
{
	if (customer_animator)
		customer_animator->PlayClip(ResourceFinder::GetGnomeAnimationClipName(type));
}

void CustomerAI::OnSuccess()
{
	GameManagerComponent& gm = GameManager::GetGM();
	ChangeState(CustomerStateType::Run);
	float t = static_cast<OrderState*>(state_map[CustomerStateType::Order])->t;
	int tip;
	const auto& timeData = CustomerSpawner::GetTimeData(gm.GetCurrentStageNum());
	if (t < timeData.AngerTime) //분노
		tip = 0;
	if (t < timeData.NervousTime)//초조
		tip = 10;
	else
		tip = 25;
	GameManager::GetGM().AddScore(wanted_potion, tip);
	isOrderEnd = true;



	auto& tb = GetComponent<TextBubble>();
	tb.ShowBubble();
	int i = Random::Range(10, 13);
	tb.SetBubbleText(StringResource::GetTutorialText(std::format(L"CT_{}", i)));

	TimeSystem::Time.DelayedInvok([&tb]() {
		tb.HideBubble();
								  }, TextBubble::text_bubble_duration_common);
}

void CustomerAI::OnFailed()
{
	isOrderEnd = true;
	ChangeState(CustomerStateType::Run);

	auto& tb = GetComponent<TextBubble>();
	tb.ShowBubble();
	int i = Random::Range(14, 15);
	tb.SetBubbleText(StringResource::GetTutorialText(std::format(L"CT_{}", i)));

	TimeSystem::Time.DelayedInvok([&tb]() {
		tb.HideBubble();
								  }, TextBubble::text_bubble_duration_common);
}

void CustomerAI::SaveVFX()
{
	using namespace Binary;
	if (!std::filesystem::exists(VFX_HitDataPath))
	{
		std::filesystem::create_directories(std::filesystem::path(VFX_HitDataPath).parent_path());
	}
	std::ofstream ofs(VFX_HitDataPath, std::ios::binary | std::ios::trunc);
	{
		constexpr size_t Version = 0;
		Write::data(ofs, Version);

		Write::string(ofs, hit);
	}
	ofs.close();
}

void CustomerAI::LoadVFX()
{
	using namespace Binary;
	std::ifstream ifs(VFX_HitDataPath, std::ios::binary);
	if (ifs.is_open())
	{
		size_t Version = 0;
		Version = Read::data<size_t>(ifs);

		if (Version == 0)
		{
			hit = Read::string(ifs);
		}
	}
	ifs.close();
}