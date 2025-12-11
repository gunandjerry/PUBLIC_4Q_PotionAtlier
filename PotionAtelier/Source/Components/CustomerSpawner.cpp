#include "CustomerSpawner.h"
#include <Core/TimeSystem.h>
#include <Utility/ImguiHelper.h>
#include <Utility/SerializedUtility.h>
#include <Utility/ExceptionUtility.h>
#include <Thread/ThreadPool.h>
#include <Object/GnomeCustomer.h>
#include <Object/MageCustomer.h>
#include <Scene/Base/Scene.h>

#include "Object/GameManager.h"
#include "Components/GameManagerComponent.h"
#include <Components/WaypointComponent.h>
#include "Components/CustomerAI.h"
#include "Components/FlyingGnome.h"

GnomeCustomer* CustomerSpawner::FindFirstOrderGnome()
{
	GameObject* spawnerObj = GameObject::Find(L"SpawnerObject");
	GnomeCustomer* firstGnome = nullptr;
	if (spawnerObj)
	{
		int minOrder = std::numeric_limits<int>::max();
		CustomerSpawner& spawner = spawnerObj->GetComponent<CustomerSpawner>();
		for (auto& gnome : spawner.gnomePoolvec)
		{
			if (gnome->Active)
			{ 
				CustomerAI* ai = gnome->IsComponent<CustomerAI>();
				if (ai && !ai->isOrderEnd)
				{
					const int& gnomeQueIndex = ai->myOrderQueueIndex;
					if (0 <= gnomeQueIndex && minOrder > gnomeQueIndex)
					{
						firstGnome = gnome;
						minOrder = gnomeQueIndex;
					}
				}	
			}
		}
	}
	return firstGnome;
}

CustomerSpawner::CustomerSpawner()
{
	
}

CustomerSpawner::~CustomerSpawner()
{

}

void CustomerSpawner::Serialized(std::ofstream& ofs)
{
	using namespace Binary;

	Write::Vector3(ofs, transform.position);	// 현재 위치를 spawn_position으로

}

void CustomerSpawner::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;

	spawn_position = Read::Vector3(ifs);
	if (spawn_position.y < 4.0f) 
		spawn_position.y = 4.0f;

}

void CustomerSpawner::InspectorImguiDraw()
{
	ImGui::Text((const char*)u8"최대 등장 가능한 노움");
	bool isChange = false;
	isChange |= ImGui::InputInt((const char*)u8"1 스테이지", &maxCustomerSpawnCount[1]);
	isChange |= ImGui::InputInt((const char*)u8"2 스테이지", &maxCustomerSpawnCount[2]);
	isChange |= ImGui::InputInt((const char*)u8"3 스테이지", &maxCustomerSpawnCount[3]);

	if (ImGui::TreeNode((const char*)u8"인내 게이지"))
	{
		for (size_t i = 1; i <= 3; i++)
		{
			ImGui::PushID(i);
			ImGui::Text((const char*)u8"%d 스테이지 노움 인내 게이지 시간 설정", i);
			isChange |= ImGui::InputFloat((const char*)u8"평온", &timeData[i].CalmTime);
			isChange |= ImGui::InputFloat((const char*)u8"초조", &timeData[i].NervousTime);
			isChange |= ImGui::InputFloat((const char*)u8"분노", &timeData[i].AngerTime);
			ImGui::Text((const char*)u8"총 합 : %f", timeData[i].CalmTime + timeData[i].NervousTime + timeData[i].AngerTime);
			ImGui::Text("");
			ImGui::PopID();		
		}
		ImGui::TreePop();
	}

	if (isChange)
	{
		SaveMaxCustomerSpawnCount();
	}

#ifdef _DEBUG
	if (ImGui::Button((const char*)u8"투더문"))
	{
		if (GnomeCustomer* firstGnome = FindFirstOrderGnome())
		{
			firstGnome->GetComponent<FlyingGnome>().FlyMeToTheMoon();
		}
		else
			Debug_printf("없어\n");
	}
#endif // _DEBUG
}

constexpr const wchar_t* MaxCustomerDataPath = L"Resource/CustomerData/MaxCustomerData.BinaryData";
constexpr const wchar_t* TimeGageDataPath = L"Resource/CustomerData/TimeGageData.BinaryData";
void CustomerSpawner::SaveMaxCustomerSpawnCount()
{
	//데이터
	{
		std::filesystem::path outPath(MaxCustomerDataPath);
		if (!std::filesystem::exists(outPath))
		{
			std::filesystem::create_directories(outPath.parent_path());
		}

		std::ofstream ofs(outPath, std::ios::binary | std::ios::trunc);
		{
			for (auto& item : maxCustomerSpawnCount)
			{
				Binary::Write::data(ofs, item);
			}
		}
		ofs.close();
	}

	std::filesystem::path outPath(TimeGageDataPath);
	if (!std::filesystem::exists(outPath))
	{
		std::filesystem::create_directories(outPath.parent_path());
	}

	std::ofstream ofs(outPath, std::ios::binary | std::ios::trunc);
	{
		constexpr size_t Version = 0;
		Binary::Write::data(ofs, Version);

		for (auto& data : timeData)
		{
			Binary::Write::data(ofs, data.CalmTime);
			Binary::Write::data(ofs, data.NervousTime);
			Binary::Write::data(ofs, data.AngerTime);
		}
	}
	ofs.close();
}

void CustomerSpawner::LoadMaxCustomerSpawnCount()
{
	{
		std::ifstream ifs(MaxCustomerDataPath, std::ios::binary);
		if (ifs.is_open())
		{
			for (auto& item : maxCustomerSpawnCount)
			{
				item = Binary::Read::data<int>(ifs);
			}
		}
		ifs.close();
	}
	{
		std::ifstream ifs(TimeGageDataPath, std::ios::binary);
		if (ifs.is_open())
		{
			size_t Version = 0;
			Version = Binary::Read::data<size_t>(ifs);

			for (auto& data : timeData)
			{
				data.CalmTime    = Binary::Read::data<float>(ifs);
				data.NervousTime = Binary::Read::data<float>(ifs);
				data.AngerTime   = Binary::Read::data<float>(ifs);
			}
		}
		ifs.close();
	}
}

void CustomerSpawner::Awake()
{
	LoadMaxCustomerSpawnCount();
}

void CustomerSpawner::Start()
{
	isInit = false;

	using namespace Utility;
	GnomeCounter = 0;

	current_stage = GameManager::GetGM().GetCurrentStageNum();

	std::wstring filename = L"Resource/CustomerData/Interval_Offset_Data_Table.csv";
	ReadIntervalDataTable(filename);

	/*filename = L"Resource/CustomerData/Magic_Data_Table.csv";
	ReadMageDataTable(filename);*/

	adjusted_respawn_interval = initial_respawn_interval;

	if (duration_end_index == current_spawn_index)
	{
		if (spawndata_index < static_cast<int>(spawndata_table.size()))
		{
			duration_start_index = spawndata_table[spawndata_index].start_index;
			duration_end_index = duration_start_index + spawndata_table[spawndata_index].duration_index;
			offset = spawndata_table[spawndata_index].offset;
		}
		++spawndata_index;

		if (duration_start_index <= current_spawn_index && current_spawn_index < duration_end_index)
		{
			adjusted_respawn_interval = std::max<float>(initial_respawn_interval + offset, 1.f);
		}
		else
		{
			adjusted_respawn_interval = initial_respawn_interval;
		}
	}

	if (GameObject* trail = sceneManager.FindObject(L"CustomerTrail"))
	{
		spawn_position = trail->GetComponent<WaypointComponent>().GetWaypoints().front().position;
	}

	gnomePoolvec.clear();
	for (size_t i = 0; i < maxCustomerSpawnCount[current_stage]; i++)
	{
		gnomePoolvec.emplace_back((GnomeCustomer*)gameObjectFactory.DeserializedObject(L"Resource/GameObject/GnomeCustomer.GameObject"));
		gnomePoolvec.back()->Active = false;
	}
	TimeSystem::Time.UpdateTime();

	isInit = true;
}
#include <chrono>
void CustomerSpawner::Update()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay())
		return;
#endif // _EDITOR
	if (GameManager::GetGM().IsTimeEnd() || !GameManager::GetGM().IsStageStart())
		return;

	//최대 소환 제한
	if (maxCustomerSpawnCount[GameManager::GetGM().GetCurrentStageNum()] == GnomeCounter)
		return;

	if(timer < adjusted_respawn_interval)
		timer += TimeSystem::Time.DeltaTime;

	if (GameManager::GetGM().IsFever())
		return;

	if (timer >= adjusted_respawn_interval)
	{
		timer = fmod(timer, adjusted_respawn_interval);

		if (mage_index < static_cast<int>(mage_spawndata_table.size()) && current_spawn_index == mage_spawndata_table[mage_index])
		{					
			++mage_index; //?
		}
		else
		{
			for (auto& gnome : gnomePoolvec)
			{
				if (gnome->Active == false)
				{
					gnome->Active = true;
					gnome->transform.position = spawn_position;
					gnome->GetComponent<CustomerAI>().ChangeState(CustomerStateType::Run);
					GnomeCounter++;
					break;
				}
			}
		}
		++current_spawn_index;

		// 다음 table 읽기에 대한 처리
		if (duration_end_index == current_spawn_index)
		{
			if (spawndata_index < static_cast<int>(spawndata_table.size()))
			{
				duration_start_index = spawndata_table[spawndata_index].start_index;
				duration_end_index = duration_start_index + spawndata_table[spawndata_index].duration_index;
				offset = spawndata_table[spawndata_index].offset;
			}
			++spawndata_index;
		}

		if (duration_start_index <= current_spawn_index && current_spawn_index < duration_end_index)
		{
			adjusted_respawn_interval = std::max<float>(initial_respawn_interval + offset, 1.f);
		}
		else
		{
			adjusted_respawn_interval = initial_respawn_interval;
		}
	}
}

void CustomerSpawner::ReadIntervalDataTable(const std::wstring& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
	{
		__debugbreak();
		return;
	}

	char bom[3]{};
	file.read(bom, 3);

	//std::cout << "BOM: " << std::hex << (int)(unsigned char)bom[0] << " "
	//	<< (int)(unsigned char)bom[1] << " "
	//	<< (int)(unsigned char)bom[2] << std::dec << std::endl;

	file.clear();
	file.seekg(0, std::ios::beg);

	if (bom[0] == (char)0xEF && bom[1] == (char)0xBB && bom[2] == (char)0xBF)
	{
		ReadIntervalDataTable_UTF8(file);
	}
	else if (bom[0] == (char)0xFF && bom[1] == (char)0xFE)
	{
		file.close();
		std::wifstream wfile(filename, std::ios::binary | std::ios::in);
		if (!wfile.is_open())
		{
			__debugbreak();
			return;
		}
		wfile.seekg(1);
		ReadIntervalDataTable_UTF16(wfile);
	}
	else if (bom[0] == (char)0xFE && bom[1] == (char)0xFF)
	{
		file.close();
		std::wifstream wfile(filename, std::ios::binary | std::ios::in);
		if (!wfile.is_open())
		{
			__debugbreak();
			return;
		}
		wfile.seekg(1);
		ReadIntervalDataTable_UTF16(wfile);
	}
	else
	{
		// "Assuming ASCII or UTF-8 without BOM"
		ReadIntervalDataTable_UTF8(file);
	}
}

void CustomerSpawner::ReadIntervalDataTable_UTF8(std::ifstream& file)
{
	std::string line;
	std::getline(file, line);	// 헤더 Stage, Start_Index, Duration_Index, Offset
	
	while (std::getline(file, line))
	{
		std::stringstream ss(line);
		SpawnData data{};
		std::string value;

		if (std::getline(ss, value, ','))
		{
			int stage{};
			std::istringstream(value) >> stage;
			if (stage == current_stage)
			{
				std::getline(ss, value, ',');
				const int index = std::stoi(value);
				data.start_index = index;		// 배열 index에 맞게 조정
				duration_end_index = std::min(duration_end_index, index);

				std::getline(ss, value, ',');
				data.duration_index = std::stoi(value);

				std::getline(ss, value, ',');
				data.offset = std::stof(value);

				spawndata_table.push_back(data);
			}
		}
	}
}

void CustomerSpawner::ReadIntervalDataTable_UTF16(std::wifstream& file)
{
	std::wstring line;
	std::getline(file, line);	// 헤더 Stage, Start_Index, Duration_Index, Offset

	while (std::getline(file, line))
	{
		std::wstringstream wss(line);
		SpawnData data{};
		std::wstring value;

		if (std::getline(wss, value, L','))
		{
			int stage{};
			std::wistringstream(value) >> stage;
			if (stage == current_stage)
			{
				std::getline(wss, value, L',');
				int index = std::stoi(value);
				data.start_index = index;		// 배열 Index에 맞게 조정

				std::getline(wss, value, L',');
				data.duration_index = std::stoi(value);

				std::getline(wss, value, L',');
				data.offset = std::stof(value);

				spawndata_table.push_back(data);
			}
		}
	}
}

void CustomerSpawner::ReadMageDataTable(const std::wstring& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
	{
		__debugbreak();
		return;
	}

	char bom[3];
	file.read(bom, 3);

	file.clear();
	file.seekg(0);

	if (bom[0] == (char)0xEF && bom[1] == (char)0xBB && bom[2] == (char)0xBF)
	{
		// UTF8
		ReadMageDataTable_UTF8(file);
	}
	else if (bom[0] == (char)0xFF && bom[1] == (char)0xFE)
	{
		// UTF16_LE
		std::wifstream file(filename, std::ios::binary);
		file.seekg(2);
		ReadMageDataTable_UTF16(file);
	}
	else if (bom[0] == (char)0xFE && bom[1] == (char)0xFF)
	{
		// UTF16_BE
		std::wifstream file(filename, std::ios::binary);
		file.seekg(2);
		ReadMageDataTable_UTF16(file);
	}
	else
	{
		// Unknown "이 파일은 또 뭘까...."
	}
}

void CustomerSpawner::ReadMageDataTable_UTF8(std::ifstream& file)
{
	std::string line;
	std::getline(file, line);	// 헤더 Stage, Index

	while (std::getline(file, line))
	{
		std::stringstream ss(line);
		std::string value;

		if (std::getline(ss, value, ','))
		{
			int stage{};
			std::istringstream(value) >> stage;
			if (stage == current_stage)
			{
				std::getline(ss, value, ',');
				const int index = std::stoi(value);
				mage_spawndata_table.push_back(index);		// 배열 index에 맞게 조정
			}
		}
	}
}

void CustomerSpawner::ReadMageDataTable_UTF16(std::wifstream& file)
{
	std::wstring line;
	std::getline(file, line);	// 헤더 Stage, Index

	while (std::getline(file, line))
	{
		std::wstringstream wss(line);
		std::wstring value;

		if (std::getline(wss, value, L','))
		{
			int stage{};
			std::wistringstream(value) >> stage;
			if (stage == current_stage)
			{
				std::getline(wss, value, L',');
				const int index = std::stoi(value);
				mage_spawndata_table.push_back(index);
			}
		}
	}
}

