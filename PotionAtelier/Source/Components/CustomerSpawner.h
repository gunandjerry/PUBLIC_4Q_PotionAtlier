#pragma once
#include <Component/Base/Component.h>
#include <EventDelegate.h>

class GameObject;

struct SpawnData
{
	int start_index;
	int duration_index;
	float offset;
};

class GnomeCustomer;
class CustomerSpawner : public Component
{
	inline static struct TimeData
	{
		//평온 시간
		float CalmTime = 10.f;
		//초조 시간
		float NervousTime = 10.f;
		//분노 시간
		float AngerTime = 10.f;
	}
	timeData[4];
	inline static bool isInit;
public:
	static const TimeData& GetTimeData(int index) { return timeData[index]; }
	inline static int GnomeCounter = 0;
	static GnomeCustomer* FindFirstOrderGnome();
	static bool IsInit() { return isInit; }
public:
	CustomerSpawner();
	virtual ~CustomerSpawner() override;

	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
	virtual void InspectorImguiDraw() override;

public:
	virtual void Awake() override;
	virtual void Start() override;

protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override;
	virtual void LateUpdate() override {}

private:
	void ReadIntervalDataTable(const std::wstring& filename);
	void ReadIntervalDataTable_UTF8(std::ifstream& file);
	void ReadIntervalDataTable_UTF16(std::wifstream& file);

	void ReadMageDataTable(const std::wstring& filename);
	void ReadMageDataTable_UTF8(std::ifstream& file);
	void ReadMageDataTable_UTF16(std::wifstream& file);

private:
	int current_stage = 1;
	Vector3 spawn_position;
	float initial_respawn_interval = 10.0f;
	float adjusted_respawn_interval{};
	float timer = 0.0f;
	int current_spawn_index = 0;
	int spawndata_index = 0;
	int mage_index = 0;
	int duration_start_index = 0;
	int duration_end_index = 9999;
	float offset = 0;

	std::vector<SpawnData>		spawndata_table;
	std::vector<int>			mage_spawndata_table;
	std::vector<GnomeCustomer*> gnomePoolvec;

	//스테이지별 최대 노움
	int maxCustomerSpawnCount[4]{};
	void SaveMaxCustomerSpawnCount();
	void LoadMaxCustomerSpawnCount();
};