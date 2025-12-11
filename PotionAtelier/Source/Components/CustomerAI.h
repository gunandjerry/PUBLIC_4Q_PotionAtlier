#pragma once
#include <unordered_map>
#include <Math/Mathf.h>
#include <Components/WaypointComponent.h>
#include <CustomerState.h>
#include <HoldableTypes.h>
#include <ResourceFinder.h>

enum class Customer
{
	Gnome,
	Mage
};

class CustomerAI : public Component
{
	friend class GnomeCustomer;
	friend class FlyingGnome;

public:
	CustomerAI();
	virtual ~CustomerAI() override;

public:
	virtual void Awake() override;
	virtual void Start() override;

	virtual void Serialized(std::ofstream& ofs) override {}
	virtual void Deserialized(std::ifstream& ifs) override {}
	virtual void InspectorImguiDraw() override;

protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override;
	virtual void LateUpdate() override {}

public:
	void ResetFlag();
	void ChangeState(CustomerStateType new_state);
	void ChangePrevState();
	CustomerStateType GetPrevStateType() const { return prev_state_type; }

	bool RayForward();
	void Move();
	void MoveSort();
	bool IsStopPoint() const { return waypoints[std::max(current_waypoint_index, 0)].is_stop_point; }
	void FindAnimationObject(Transform* parent);
	void SetCustomerType(Customer type) { customer_type = type; }
	void SetAnimationClip(GnomeAnimType type);
public:
	void OnSuccess();
	void OnFailed();
	//std::function<void()> Success;
	//std::function<void()> Fail;
	int GetOrderID() const { return myOrderQueueIndex; }
	PotionType GetPotion() const { return wanted_potion; }
	CustomerStateType GetCurrentState() { return curr_state_type; }

	bool isOrderEnd = false;
	class CharacterController* controller = nullptr;
	float max_patience = 20.f;

	bool isSortEnd = true;
	int myOrderQueueIndex = -1;

private:
	void AssignPotionBasedOnStage();

	void SaveVFX();
	void LoadVFX();

	Customer customer_type = Customer::Gnome;
	CustomerStateType prev_state_type = CustomerStateType::Run;
	CustomerStateType curr_state_type = CustomerStateType::Run;
	CustomerState* current_state{};
	std::unordered_map<CustomerStateType, CustomerState*> state_map;
	PotionType wanted_potion = PotionType::FailurePotion;

	class TransformAnimation* customer_animator = nullptr;
	std::vector<WaypointData> waypoints;

	class ParticleSpawnComponent* VFX_Hit{ nullptr };
	inline static constexpr const wchar_t* VFX_HitDataPath = L"Resource/VFX/VFX_Hit.BinaryData";
	std::string hit{};

	int potion = 0;
	int waypoints_count = 0;
	int current_waypoint_index = -1;

	float max_speed = 10.f;
	float current_speed = 0.f;
	Vector3 separated_interact_direction{ 1, 0, 0 };
	float superficial_rotation{ 0.0f };
	float rotate_speed{ 5.0f };

	float shot_ray_interval = 0.1f;
	float ray_distance = 15.0f;
	Vector2 ray_size{ 0.5f, 0.5f };



	/* Punched */
	bool punched{ false };
};