#include "GameManagerComponent.h"
#include <framework.h>
#include <string>
#include "Object/TextObject.h"
#include "Object/TutorialManager.h"
#include "Components/TutorialManagerComponent.h"
#include "Components/DeliverCounter.h"

#include "Object/RecipeManager.h"
#include "Object/AudioPlayerObject.h"
#include "Components/BoingBoing.h"
#include "Object/ButtonObject/GameStartButton.h"
#include "Object/BackGroundMusicObject.h"
#include "Object/GameManager.h"


GameManagerComponent::GameManagerComponent() = default;

GameManagerComponent::~GameManagerComponent()
{
	ClearUIObjects();
}

void GameManagerComponent::InspectorImguiDraw()
{
	ImGui::PushID("GameManagerComponent");
#if _EDITOR
	for (auto& [name, func] : editorFuncButton)
	{
		if(ImGui::Button(name.c_str()))
		{
			func();
		}
	}
#endif // _EDITOR

	ImGui::Text("Deubg");
	ImGui::Text("Stage Start : %s", isStageStart ? "true" : "false");
	ImGui::Text("Stage Clear : %s", isStageClear ? "true" : "false");
	ImGui::Text("Is TimeOut : %s", isTimeEnd ? "true" : "false");
	if (ImGui::Button("Reset Flags"))	{ ResetFlags(); }
	if (ImGui::Button((const char*)u8"메인 메뉴로")) { MainMenuScene(); }
	if (ImGui::Button((const char*)u8"히든 별 획득")) { UnlockSecret(); };
	if (ImGui::Button("Restart Stage")) { StageRestart(); }
	if (ImGui::Button("Test Fever")) { FeverStart(); }
	if (ImGui::Button("Quick Start")) { isStageStart = true; }
	ImGui::InputInt("Stage Num", &stageNum);
	if (ImGui::TreeNode((const char*)u8"스테이지 정보"))
	{
		ImGui::InputFloat((const char*)u8"제한 시간", &PlayTime);
		int i = 0;
		for (auto& item : starScores)
		{
			ImGui::PushID(++i);
			ImGui::InputInt(std::format("{}{}", i, (const char*)u8"번째 별 목표 점수").c_str(), &item);
			ImGui::PopID();
		}
		ImGui::InputInt((const char*)u8"히든 별 목표 점수", &trophyScore);

		ImGui::InputFloat((const char*)u8"최대 피버 게이지", &FeverMaxPercent);
		ImGui::InputFloat((const char*)u8"주문 처리당 차는 게이지", &FeverPercent);
		ImGui::InputFloat((const char*)u8"피버 지속시간", &FeverMaxTime);
		if (ImGui::Button(std::format("{} {}", stageNum, (const char*)u8"스테이지 정보 저장하기").c_str()))
		{
			SaveStageData();
		}
		if (ImGui::Button(std::format("{} {}", stageNum, (const char*)u8"스테이지 정보 로드하기").c_str()))
		{
			LoadStageData();
		}
		ImGui::TreePop();
	}
	{
		static int addScore = 0;
		ImGui::Text("Total Score : %d", GetTotalScore());
		ImGui::InputInt("Add Score", &addScore);
		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			AddScore(addScore);
		}
		ImGui::Text("Score : %d", GetScore());
		ImGui::Text("Tip : %d", GetTip());
	}
	ImGui::Text((const char*)u8"피버 게이지 %f", GetFeverGaugePercentage());
	ImGui::Text("Last Score : %d", lastScore);
	ImGui::Text((const char*)u8"얻은 별 개수 : %d", GetStarCount());
	ImGui::Text((const char*)u8"제한 시간 : %f", PlayTime);
	ImGui::Text((const char*)u8"현재 시간 : %f", uiElapsedTimes[E_UIName::TimerUI]);
	ImGui::Text((const char*)u8"남은 시간 : %f", GetStageRemainingTime());
	ImGui::DragFloat((const char*)u8"배속", &TimeSystem::Time.timeScale, 0.1f, Mathf::Epsilon, 10.f);

	if(ImGui::TreeNode((const char*)u8"주문 하기"))
	{
		UINT potion = 1 << 1;
		while (potion != PotionType::RainbowPotion)
		{
			PotionType type = PotionType(potion);
			if(ImGui::Button(GetPotionName(type)))
			{
				OrderPotion(type, 
					[this, type]{ AddScore(type, 0); },
					[]{});
			}
			potion = potion << 1;
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode((const char*)u8"서빙 하기"))
	{
		UINT potion = 1 << 1;
		while (potion != PotionType::RainbowPotion)
		{
			PotionType type = PotionType(potion);
			if (ImGui::Button(GetPotionName(type)))
			{
				ServeOrder(type);
			}
			potion = potion << 1;
		}
		ImGui::TreePop();
	}

	ImGui::InputText("VFX_Coin", (char*)coin.c_str(), coin.size(), ImGuiInputTextFlags_CallbackResize,
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
		}, &coin);

	ImGui::InputText("VFX_FeverBubble", (char*)fever_bubble.c_str(), fever_bubble.size(), ImGuiInputTextFlags_CallbackResize,
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
		}, & fever_bubble);

	ImGui::InputText("VFX_FeverMagic", (char*)fever_magic.c_str(), fever_magic.size(), ImGuiInputTextFlags_CallbackResize,
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
		}, & fever_magic);

	ImGui::InputText("VFX_Lightening", (char*)lightening.c_str(), lightening.size(), ImGuiInputTextFlags_CallbackResize,
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
		}, & lightening);

	ImGui::InputText("VFX_DuringFever", (char*)during_fever.c_str(), during_fever.size(), ImGuiInputTextFlags_CallbackResize,
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
		}, & during_fever);

#ifdef _EDITOR
	static int UiSelectButtonId = 0;
	if (ImGui::TreeNode((const char*)u8"포션별 가격 설정"))
	{
		UINT potion = 1 << 1;
		for (auto& i : potionScore)
		{
			PotionType type = PotionType(potion);
			ImGui::PushID(UiSelectButtonId++);
			ImGui::Text(GetPotionName(type));
			ImGui::SameLine();
			if (ImGui::InputInt("Score", &i))
			{
				SavePotionScoreData();
			}
			ImGui::PopID();
			potion = potion << 1;
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode((const char*)u8"스테이지별 포션 가중치"))
	{
		bool change = false;
		ImGui::PushID("1");
		ImGui::Text((const char*)u8"1 스테이지");
		{
			int i = 0;
			static int potionStringIndex[_ARRAYSIZE(potionWeight1)]{};
			for (auto& item : potionWeight1)
			{
				potionStringIndex[i] = RecipeManagerComponent::GetPotionTypeIndex(item.first);
				ImGui::PushID(i);
				if (ImGui::Combo(std::format("Potion {}", i).c_str(), &potionStringIndex[i],
					RecipeManagerComponent::GetPotionListString().data(), RecipeManagerComponent::GetPotionListString().size()))
				{
					item.first = (PotionType)(1 << (potionStringIndex[i] + 1));
					change |= true;
				}
				change |= ImGui::InputInt("Weight", &item.second);
				i++;
				ImGui::PopID();
			}
		}
		ImGui::PopID();
		ImGui::PushID("2");
		ImGui::Text((const char*)u8"2 스테이지");
		{
			int i = 0;
			static int potionStringIndex[_ARRAYSIZE(potionWeight2)]{};
			for (auto& item : potionWeight2)
			{
				potionStringIndex[i] = RecipeManagerComponent::GetPotionTypeIndex(item.first);
				ImGui::PushID(i);
				if (ImGui::Combo(std::format("Potion {}", i).c_str(), &potionStringIndex[i],
					RecipeManagerComponent::GetPotionListString().data(), RecipeManagerComponent::GetPotionListString().size()))
				{
					item.first = (PotionType)(1 << (potionStringIndex[i] + 1));
					change |= true;
				}
				change |= ImGui::InputInt("Weight", &item.second);
				i++;
				ImGui::PopID();
			}
		}
		ImGui::PopID();
		ImGui::PushID("3");
		ImGui::Text((const char*)u8"3 스테이지");
		{
			int i = 0;
			static int potionStringIndex[_ARRAYSIZE(potionWeight3)]{};
			for (auto& item : potionWeight3)
			{
				potionStringIndex[i] = RecipeManagerComponent::GetPotionTypeIndex(item.first);
				ImGui::PushID(i);
				if (ImGui::Combo(std::format("Potion {}", i).c_str(), &potionStringIndex[i],
					RecipeManagerComponent::GetPotionListString().data(), RecipeManagerComponent::GetPotionListString().size()))
				{
					item.first = (PotionType)(1 << (potionStringIndex[i] + 1));
					change |= true;
				}
				change |= ImGui::InputInt("Weight", &item.second);
				i++;
				ImGui::PopID();
			}
		}
		ImGui::PopID();
		if (change)
			SavePotionWeightData();

		ImGui::TreePop();
	}
	auto UiSelectFunc = [this](std::wstring& path)
		{
			ImGui::PushID(UiSelectButtonId++);
			if (ImGui::Button("Set"))
			{
				std::filesystem::path openPath = WinUtility::GetOpenFilePath(L"GameObject");
				if (!openPath.empty())
				{
					if (openPath.is_absolute())
					{
						openPath = std::filesystem::relative(openPath, std::filesystem::current_path());
					}
					if (*openPath.begin() != L"EngineResource")
					{
						SQLiteLogger::EditorLog("Log", "GameManager Log: The UI root directory only uses \"EngineResource\".");
					}
					else
					{
						path = openPath;
						this->SaveGameManagerData();
					}
				}
			}
			ImGui::PopID();
		};
	for (int i = 0; i < UiArrayCount; i++)
	{
		auto& [path, object] = uiArray[i];
		UiSelectFunc(path);
		ImGui::SameLine();
		ImGui::Text("%s : %s", E_UIName::GetUINameToString(E_UIName::UIName(i)), utfConvert::wstring_to_utf8(path).c_str());
	}
	UiSelectButtonId = 0;
#endif // _EDITOR
	ImGui::PopID();
}

void GameManagerComponent::Awake()
{
	potionWeight0 = { PotionType::HealthPotion, 1 };

	potionWeight1[0] = { PotionType::HealthPotion, 50 };
	potionWeight1[1] = { PotionType::ManaPotion,   50 };

	potionWeight2[0] = { PotionType::HealthPotion,    50 };
	potionWeight2[1] = { PotionType::ManaPotion,      30 };
	potionWeight2[2] = { PotionType::SeductionPotion, 15 };
	potionWeight2[3] = { PotionType::AddictionPotion, 5 };

	potionWeight3[0] = { PotionType::HealthPotion,    30 };
	potionWeight3[1] = { PotionType::ManaPotion,      10 };
	potionWeight3[2] = { PotionType::SeductionPotion, 10 };
	potionWeight3[3] = { PotionType::AddictionPotion, 10 };
	potionWeight3[4] = { PotionType::FlamePotion,     10 };
	potionWeight3[5] = { PotionType::FrozenPotion,    10 };

#if _EDITOR
	using namespace std::string_literals;
	editorFuncButton.push_back(std::make_pair("Load GameManager Data"s, [this](){ LoadGameManagerData(); }));
#endif // _EDITOR
	LoadPotionScoreData();
	LoadPotionWeightData();
}

void GameManagerComponent::Start()
{
	//그냥 게임매니져 인스팩터에서 Stage Num을 0으로 하고 플레이 하면 튜토리얼 나옵니다

	counter_punch_cooltime = Random::Range(counter_punch_cooltime_min, counter_punch_cooltime_max);
}

void GameManagerComponent::Update()
{
	if (isinit) isinit1Tick = true;
	using namespace E_UIName;
	using namespace TimeSystem;
#ifdef _EDITOR
	static bool paly = false;
	if (paly != Scene::EditorSetting.IsPlay())
	{
		paly = Scene::EditorSetting.IsPlay();
		if (paly)
		{
			LoadGameManagerData();
		}
		else
		{
			Score = 0;
			ResetFlags();
			ClearUIObjects();
			memset(uiElapsedTimes, 0.f, sizeof(uiElapsedTimes));
		}
	}
	if (!paly)
		return;
#endif // _EDITOR
	for (size_t i = 0; i < UiArrayCount; i++)
	{
		hasUiObject[i] = uiArray[(UIName)i].second != nullptr;
	}
	UpdateStageStartEvent();
	UpdateTimerEvent();
	UpdateOrderSheetAnimation();
	UpdateFever();

	
	//테스트임
	//OrderSheetUpdate(0, 1 - GetProgressPercentage());
	if (isStageStart && stageNum > 0)
	{
		if (PlayTime - uiElapsedTimes[TimerUI] >= 10.0f)
		{
			counter_punch_elapsed_time += Time.DeltaTime;
			if (counter_punch_elapsed_time >= counter_punch_cooltime)
			{
				counter_punch_elapsed_time -= counter_punch_cooltime;
				GameObject* Counter = GameObject::Find(L"CounterObject");
				if (Counter != nullptr)
				{
					DeliverCounter* dc = Counter->IsComponent<DeliverCounter>();
					if (dc != nullptr)
					{
						dc->ShowFace();
						counter_punch_cooltime = Random::Range(counter_punch_cooltime_min, counter_punch_cooltime_max);
					}
				}
			}
		}
	}
}

void GameManagerComponent::AddFeverPercent()
{
	if (IsFever()) return;

	CurrentFeverPercent += FeverPercent;
	if (CurrentFeverPercent >= FeverMaxPercent)
	{
		FeverStart();
	}
}

void GameManagerComponent::FeverStart()
{
	FeverTime = 0.f;
	isFever = true;
	if (SFX_Fever) SFX_Fever->Play();
	if (VFX_FeverBubble) VFX_FeverBubble->CreateParticle();
	if (VFX_Lightening) VFX_Lightening->isSpawnParticlesByTime = true;
	if (VFX_DuringFever) VFX_DuringFever->isSpawnParticlesByTime = true;
	for (auto& item : OrderSheetVec)
	{
		Vector3 start = item.StuffPaperUI->localPosition;
		Vector3 end = item.StuffPaperUI->localPosition - Vector3::Up * 0.5f;
		SetOrderSheetAnime(item.StuffPaperUI, start, end, 0.4f);
		item.StuffPaperUI->localPosition = start;
		Transform* PotionIcon = item.PotionIcon;
		switch (item.GetPotionType())
		{
		case HealthPotion:
			PotionIcon->GetChild(0)->gameObject.Active = false;
			break;
		case ManaPotion:
			PotionIcon->GetChild(1)->gameObject.Active = false;
			break;
		case AddictionPotion:
			PotionIcon->GetChild(2)->gameObject.Active = false;
			break;
		case SeductionPotion:
			PotionIcon->GetChild(3)->gameObject.Active = false;
			break;
		case FlamePotion:
			PotionIcon->GetChild(4)->gameObject.Active = false;
			break;
		case FrozenPotion:
			PotionIcon->GetChild(5)->gameObject.Active = false;
			break;
		default:
			break;
		}
		PotionIcon->GetChild(6)->gameObject.Active = true;
		item.SetPatienceFill(1.f);
		item.successedCallBack = [this] 
			{
			AddScore(RainbowPotion, 20);
			};
		item.faildCallBack = []{};
	}

	if (hasUiObject[E_UIName::FeverVFX])
	{
		uiArray[E_UIName::FeverVFX].second->Active = true;
	}

	//주문서 꽉채우기
	int result = 0;
	while (result != -1)
	{
		result = OrderPotionRainbow();
	}
}

void GameManagerComponent::FeverEnd()
{
	CurrentFeverPercent = 0.f;
	FeverTime = 0.f;
	isFever = false;
	if (SFX_Fever) SFX_Fever->Stop();
	if (VFX_Lightening) VFX_Lightening->isSpawnParticlesByTime = false;
	if (VFX_DuringFever) VFX_DuringFever->isSpawnParticlesByTime = false;
	int i = 0;
	for (auto & item : OrderSheetVec)
	{
		Vector3 start = item.StuffPaperUI->localPosition;
		Vector3 end = item.StuffPaperUI->localPosition + Vector3::Up * 0.5f;
		SetOrderSheetAnime(item.StuffPaperUI, start, end, 0.4f);
		item.StuffPaperUI->localPosition = start;
		Transform* PotionIcon = item.PotionIcon;

		PotionIcon->GetChild(6)->gameObject.Active = false;
		item.UI_OrderSheet->gameObject.Active = false;
		if (!orderQueue.empty())
		{
			std::erase(orderQueue, i);
		}	
		i++;
	}

	if (hasUiObject[E_UIName::FeverVFX])
	{
		uiArray[E_UIName::FeverVFX].second->Active = false;
	}

	TimeSystem::Time.DelayedInvok(
		[this]
		{
			SortOrderSheet();
		},
		0.4f);
}

std::vector<std::pair<PotionType, int>> GameManagerComponent::GetCurrentPotionWeight() const
{
	std::vector<std::pair<PotionType, int>> vec;
	switch (stageNum)
	{
	case 1:
		for (auto& item : potionWeight1)
		{
			vec.push_back(item);
		}
		return vec;
	case 2:
		for (auto& item : potionWeight2)
		{
			vec.push_back(item);
		}
		return vec;
	case 3:
		for (auto& item : potionWeight3)
		{
			vec.push_back(item);
		}
		return vec;

	default:
	case 0:
		vec.push_back(potionWeight0);
		return vec;
	}
}

void GameManagerComponent::ClearUIObjects()
{
	for (auto& [path, object] : uiArray)
	{
		if (object)
		{
			GameObject::Destroy(object);
			object = nullptr;
		}
	}
	std::fill(std::begin(hasUiObject), std::end(hasUiObject), false);
}

void GameManagerComponent::ResetFlags()
{
	isStageClear = false;
	isStageStart = false;
	isTimeEnd = false;
	isFever = false;
	TimeSystem::Time.timeScale = 1.f;
}

void GameManagerComponent::UpdateStageStartEvent()
{
	if (!isinit)
	{
		isinit = true;
		std::wstring wstr = std::wstring(coin.begin(), coin.end());
		if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
		{
			VFX_Coin = findItem->IsComponent<ParticleSpawnComponent>();
		}
		wstr = std::wstring(fever_bubble.begin(), fever_bubble.end());
		if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
		{
			VFX_FeverBubble = findItem->IsComponent<ParticleSpawnComponent>();
		}
		wstr = std::wstring(fever_magic.begin(), fever_magic.end());
		if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
		{
			VFX_FeverMagic = findItem->IsComponent<ParticleSpawnComponent>();
		}
		wstr = std::wstring(lightening.begin(), lightening.end());
		if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
		{
			VFX_Lightening = findItem->IsComponent<ParticleSpawnComponent>();
		}
		wstr = std::wstring(during_fever.begin(), during_fever.end());
		if (const auto& findItem = GameObject::Find<ParticleSpawnerObject>(wstr.c_str()))
		{
			VFX_DuringFever = findItem->IsComponent<ParticleSpawnComponent>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_NewsClick"))
		{
			SFX_NewsClick = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Order"))
		{
			SFX_Order = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Gold"))
		{
			SFX_Gold = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Gold_Pang"))
		{
			SFX_Gold_Pang = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Fever"))
		{
			SFX_Fever = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Result_Popup"))
		{
			SFX_Result_Popup = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Result_Star1"))
		{
			SFX_Result_Star1 = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Result_Star2"))
		{
			SFX_Result_Star2 = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Result_Star3"))
		{
			SFX_Result_Star3 = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Trophy"))
		{
			SFX_Trophy = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_ClassA"))
		{
			SFX_ClassA = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_ClassB"))
		{
			SFX_ClassB = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_ClassC"))
		{
			SFX_ClassC = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Hidden"))
		{
			SFX_Hidden = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Click"))
		{
			SFX_Click = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Ready"))
		{
			SFX_Ready = findItem->IsComponent<AudioBankClip>();
		}
		if (const auto& findItem = GameObject::Find<AudioPlayerObject>(L"SFX_Go"))
		{
			SFX_Go = findItem->IsComponent<AudioBankClip>();
		}

		auto temp = GameObject::FindFirst<GameManagerHelper>();
		if (temp)
		{
			helper = GameObject::FindFirst<GameManagerHelper>()->component;
		}
	}

	using namespace E_UIName;
	using namespace TimeSystem;
	InputManager::Input& Input = inputManager.input;
	if (hasUiObject[PotionNewsUI])
	{
		if (uiArray[PotionNewsUI].second->Active)
		{
			auto& input = inputManager.input;
			if (input.IsKeyDown(KeyboardKeys::Space))
			{
				auto object = uiArray[PotionNewsUI].second;

				if (object->Active)
				{
					if (SFX_NewsClick) SFX_NewsClick->Play();
				}
				object->Active = false;

				if (BackGroundMusicObject* bgmObject = GameObject::Find<BackGroundMusicObject>(L"BackGroundMusic"))
				{
					if (BackGroundMusicComponent* BGM = bgmObject->component)
					{
						int stageNumt = std::clamp(stageNum, 1, 3);
						auto temp =
							BGM->eventParametors
							| std::views::filter([](const BankEventParametor& parameter) { return parameter.name == "BGM_Change"; });
						if (!temp.empty())
						{
							temp.front().value = std::format("Stage{}", stageNumt);
						}
						BGM->Play();
						BGM->SetData("BGM_Change", std::format("Stage{}", stageNumt));
					}
				}


			}
			return;
		}
	}

	if (!isStageStart && !isTimeEnd)
	{
		if (hasUiObject[StageStartUI])
		{
			float& elapsedTime = uiElapsedTimes[StageStartUI];
			constexpr float spaceHoldTime = 1.f;
			GameObject* temp = (GameObject*)uiArray[StageStartUI].second;
			UIMaterialObject* chargeBar = (UIMaterialObject*)&temp->transform.GetChild(1)->gameObject;
			if (Input.IsKey(KeyboardKeys::Space) && uiArray[StageStartUI].second->Active)
			{
				elapsedTime += Time.DeltaTime;
				float t = elapsedTime / spaceHoldTime;
				chargeBar->uiComponenet.anchorMax.x = Mathf::Lerp(0, 1, t);
				chargeBar->uiComponenet.SetTransform();
				if (t >= 1.f)
				{
					uiArray[StageStartUI].second->Active = false;
					if (hasUiObject[StageUI])
						uiArray[StageUI].second->Active = false;

					if (hasUiObject[UIName::Ready])
						uiArray[UIName::Ready].second->Active = true;
					if (SFX_Ready) SFX_Ready->Play();
					Time.DelayedInvok(
						[this]()
						{
							if (hasUiObject[UIName::Ready])
								uiArray[UIName::Ready].second->Active = false;
							if (hasUiObject[UIName::Start])
								uiArray[UIName::Start].second->Active = true;
							if (SFX_Go) SFX_Go->Play();


							auto door = GameObject::Find(L"Door");
							if (door)
							{

							}
						},
						1.f
					);
					Time.DelayedInvok(
						[this]()
						{ 
							if(hasUiObject[UIName::Start])
								uiArray[UIName::Start].second->Active = false;	

							isStageStart = true; 
							if (processTutorialOnStageStart)
							{
								TutorialManager::GetInstance()->StartTutorial();
							}

							if (hasUiObject[TimerUI])
								uiArray[TimerUI].second->Active = true;
							if (hasUiObject[ScoreUI])
								uiArray[ScoreUI].second->Active = true;		
							if (hasUiObject[FeverUI])
								uiArray[FeverUI].second->Active = true;

						},
						2.f);

				}
			}
			else if(elapsedTime < spaceHoldTime)
			{
				if (hasUiObject[UIName::Ready])
					uiArray[UIName::Ready].second->Active = false;
				if (hasUiObject[UIName::Start])
					uiArray[UIName::Start].second->Active = false;

				uiArray[StageStartUI].second->Active = true;
				elapsedTime = 0.f;
				chargeBar->uiComponenet.anchorMax.x = 0.f;
				chargeBar->uiComponenet.SetTransform();
			}
		}
		if (hasUiObject[StageUI])
		{
			uiArray[StageUI].second->Active = true;
		}

	}

}

void GameManagerComponent::UpdateTimerEvent()
{
	using namespace E_UIName;
	using namespace TimeSystem;
	if (processTutorialOnStageStart)
		return;

	if (isStageStart && !isTimeEnd && uiElapsedTimes[TimerUI] < PlayTime)
	{
		uiElapsedTimes[TimerUI] += Time.DeltaTime;
		if (hasUiObject[TimerUI] && TimerText)
		{
			float remainTime = GetStageRemainingTime();
			int totalSeconds = static_cast<int>(remainTime);
			int minutes = totalSeconds / 60;
			int seconds = totalSeconds % 60;
			if (TimerText)
			{
				TimerText->text = std::to_wstring(minutes);
				TimerText->text += L":";
				TimerText->text += std::to_wstring(seconds);
				if (remainTime <= 30.f)
				{
					TimerText->Highlight(1.4f, Color{ 1,0,0,1 }, 1.0f);
					if (TimerClock)
						TimerClock->uiComponenet.RotationAnimation(10.f, 0.1f);
				}
			}
			if (TimerBar)
			{
				TimerBar->uiComponenet.anchorMax.x = Mathf::Lerp(1.f, TimerBar->uiComponenet.anchorMin.x, GetProgressPercentage());
				TimerBar->uiComponenet.GetCustomData().SetField("hue", Mathf::Lerp(-0.333f, 0.f, TimerBar->uiComponenet.anchorMax.x));
				TimerBar->uiComponenet.SetTransform();
			}
		}
		if (hasUiObject[ScoreUI] && ScoreText)
		{
			ScoreText->SetText(std::to_wstring(Score));

			//일반 별
			for (size_t i = 0; i < UI_Star_FillVec.size() - 1; i++)
			{
				UIMaterialObject* star = UI_Star_FillVec[i];
				if (starScores[i] <= Score)
				{
					if (star->Active == false)
					{
						star->Active = true;
						star->GetComponent<BoingBoingUI>().Boing(0);
						//star->uiComponenet.ScaleAnimation(1.4f, 1.f);
					}		
				}
				else
				{
					star->Active = false;
				}
			}
			//특수 도전과제
			if (GetStarCount() >= 3)
			{
				if (trophyScore <= Score)
					UnlockSecret();

				if (UI_Star_BGVec.back()->Active == false)
				{
					UI_Star_BGVec.back()->Active = true;
				}
				if (UI_Star_FillVec.back()->Active && UI_Star_FillVec.back()->uiComponenet.Enable == false)
				{
					UI_Star_FillVec.back()->uiComponenet.Enable = true;
					UI_Star_FillVec.back()->GetComponent<BoingBoingUI>().Boing(0);
					//UI_Star_FillVec.back()->uiComponenet.ScaleAnimation(1.4f, 1.f);
				}
			}		
			else
			{
				UI_Star_BGVec.back()->Active = false;
				UI_Star_FillVec.back()->uiComponenet.Enable = false;
			}
		}
		if (PlayTime <= uiElapsedTimes[TimerUI])
		{
			if (SFX_Result_Popup) SFX_Result_Popup->Play();
			Score -= FailedOrder * 10;
			lastScore = Score;
			lastStarCount = GetStarCount();
			isTimeEnd = true;
			isStageStart = false;
			uiElapsedTimes[TimerUI] = PlayTime;
			if (TimerText)
			{
				TimerText->text = std::to_wstring(0);
				TimerText->text += L":";
				TimerText->text += std::to_wstring(0);
			}
			if (hasUiObject[UIName::End])
			{
				uiArray[UIName::End].second->Active = true;
				Time.DelayedInvok(
					[this]()
					{
						uiArray[UIName::End].second->Active = false;
						if (hasUiObject[UIName::GameResult])
						{
							uiArray[UIName::GameResult].second->Active = true;					
							//페이지
							Transform* page = uiArray[UIName::GameResult].second->transform.GetChild(0);
							if (page)
							{
								Transform* textTr = uiArray[UIName::GameResult].second->transform.GetChild(3);
								if (textTr)
								{
									textTr->gameObject.Active = false;
									for (int i = 0; i < textTr->GetChildCount(); i++)
									{
										enum E_RESULT_TEXT
										{
											Total,
											//수익
											Revenue,
											//팁	
											Tip,
											//주문 성공
											SuccessOrder,
											//주문 실패
											FailOrder,
										};
										TextObject* text = (TextObject*)&textTr->GetChild(i)->gameObject;
										TextRender& textRender = text->GetComponent<TextRender>();
										switch ((E_RESULT_TEXT)i)
										{
										case E_RESULT_TEXT::Total:
											textRender.text = std::to_wstring(Score);
											break;
										case E_RESULT_TEXT::Revenue:
											textRender.text = std::to_wstring(Score - tip);
											break;
										case E_RESULT_TEXT::Tip:
											textRender.text = std::to_wstring(tip);
											break;
										case E_RESULT_TEXT::SuccessOrder:
											textRender.text = std::to_wstring(SuccessedOrder);
											break;
										case E_RESULT_TEXT::FailOrder:
											textRender.text = std::to_wstring(FailedOrder);
											break;
										default:
											break;
										}
									}
								}
							}
							///버튼
							if(Transform* button = uiArray[UIName::GameResult].second->transform.GetChild(1))
							{
								button->gameObject.Active = false;
								BoingBoingUI* boing = &button->gameObject.AddComponent<BoingBoingUI>();
								boing->use_on_ui_render2 = true;
								EventListener* eventListener = &button->gameObject.AddComponent<EventListener>();
								eventListener->SetOnClickDown(
									[boing, this, eventListener]
									{
										eventListener->SetOnClickDown([]() {});
										boing->Boing(0);
										auto event = GameObject::Find<SceneEventObject>(L"SceneEventObject");
										if (event)
										{
											auto ui = event->componenet->UI;
											if (ui)
											{
												ui->SetPosX(0);
												ui->SetPosY(0);
											}
											event->componenet->isReverse = false;
											event->componenet->EventStart();
										}
										Time.DelayedInvok(
											[this]()
											{
												if (isStageClear)
												{
													if (stageNum == 3)
													{
														//엔딩?
														EndingScene();
													}
													else
													{
														StageLoad(stageNum + 1);
													}
												}
												else
												{
													//게임 오버?
													GameOverScene();
												}
											},
											1.2f);					
									});
							}				
							//별
							for (size_t i = 0; i < uiArray[UIName::GameResult].second->transform.GetChild(2)->GetChildCount(); i++)
							{
								UIMaterialObject* star = (UIMaterialObject*)&uiArray[UIName::GameResult].second->transform.GetChild(2)->GetChild(i)->gameObject;
								star->uiComponenet.Enable = false;
							}
						}
					}
					, 1.f
				);
			}

			if (hasUiObject[UIName::TimerUI])
				uiArray[UIName::TimerUI].second->Active = false;
			if (hasUiObject[UIName::ScoreUI])
				uiArray[UIName::ScoreUI].second->Active = false;
			if (hasUiObject[UIName::OderSheetUI])
				uiArray[UIName::OderSheetUI].second->Active = false;
			if (hasUiObject[UIName::FeverUI])
			{
				for (size_t i = 0; i < uiArray[UIName::FeverUI].second->transform.GetChildCount(); i++)
				{
					uiArray[UIName::FeverUI].second->transform.GetChild(i)->gameObject.Active = false;
				}
				
				uiArray[UIName::FeverUI].second->Active = false;
			}
			if (hasUiObject[UIName::FeverVFX])
			{
				for (size_t i = 0; i < uiArray[UIName::FeverVFX].second->transform.GetChildCount(); i++)
				{
					uiArray[UIName::FeverVFX].second->transform.GetChild(i)->gameObject.Active = false;
				}

				uiArray[UIName::FeverVFX].second->Active = false;
			}

			/*목표 점수 달성*/
			if (lastScore >= starScores[0])
			{
				StageClear();
			}

		}
	}
	else
	{
		//게임 결과 UI 출력중
		if (hasUiObject[UIName::GameResult] && uiArray[UIName::GameResult].second->Active == true)
		{
			constexpr float uiDelayTime = 1.f;
			constexpr float uiAnimationTime = uiDelayTime * 4.f;
			if (uiElapsedTimes[UIName::GameResult] <= uiAnimationTime)
			{
				Transform* page = uiArray[UIName::GameResult].second->transform.GetChild(0); //페이지
				Transform* button = uiArray[UIName::GameResult].second->transform.GetChild(1); //버튼
				Transform* starsParent = uiArray[UIName::GameResult].second->transform.GetChild(2);//별들
				Transform* textParent = uiArray[UIName::GameResult].second->transform.GetChild(3); //점수
				uiElapsedTimes[UIName::GameResult] += Time.DeltaTime;
				
				if (inputManager.input.IsKeyUp(MouseKeys::leftButton) || inputManager.input.IsKeyDown(KeyboardKeys::Space))
					uiElapsedTimes[UIName::GameResult] = uiAnimationTime;

				for (int i = 0; i < 3; i++)
				{
					if (Transform* starTr = starsParent->GetChild(i))
					{
						UIMaterialObject* star = (UIMaterialObject*)&starTr->gameObject;
						if (star && star->uiComponenet.Enable == false && uiElapsedTimes[UIName::GameResult] >= uiDelayTime * (i + 1))
						{
							if (GetStarCount() >= i + 1)
							{
								star->uiComponenet.Enable = true;
								star->GetComponent<BoingBoingUI>().Boing(0);
								switch (i)
								{
								case 0:
									if (SFX_Result_Star1) SFX_Result_Star1->Play();
									break;
								case 1:
									if (SFX_Result_Star2) SFX_Result_Star2->Play();
									break;
								case 2:
									if (SFX_Result_Star3) SFX_Result_Star3->Play();
									break;
								default:
									break;
								}
							}			
							else
								uiElapsedTimes[UIName::GameResult] = uiAnimationTime;

							
						}
					}
				}
				if (Transform* starTr = starsParent->GetChild(3))
				{
					UIMaterialObject* star = (UIMaterialObject*)&starTr->gameObject;
					if (star && star->uiComponenet.Enable == false && uiElapsedTimes[UIName::GameResult] >= uiDelayTime * 4)
					{
						if (GetStarCount() == 4)
						{
							star->uiComponenet.Enable = true;
							if (SFX_Trophy) SFX_Trophy->Play();
						}
					}
				}

				if (uiElapsedTimes[UIName::GameResult] >= uiDelayTime * 4.f)
				{
					if (starsParent && starsParent->gameObject.Active == false)
						starsParent->gameObject.Active = true;

					if (button && button->gameObject.Active == false)
						button->gameObject.Active = true;		

					if (textParent && textParent->gameObject.Active == false)
						textParent->gameObject.Active = true;


					if ((SFX_ClassC && !SFX_ClassC->IsPlaying()) && (SFX_ClassB && !SFX_ClassB->IsPlaying()) && (SFX_ClassA && !SFX_ClassA->IsPlaying()))
					{
						switch (lastStarCount)
						{
						case 1:
							if (SFX_ClassC) SFX_ClassC->Play();
							break;
						case 2:
							if (SFX_ClassB) SFX_ClassB->Play();
							break;
						case 3:
							if (SFX_ClassA) SFX_ClassA->Play();
							break;
						case 4:
							if (SFX_Hidden) SFX_Hidden->Play();
							break;
						default:
							break;
						}
					}
				}
			}
			else
			{
				if (Transform* button = uiArray[UIName::GameResult].second->transform.GetChild(1))
				{
					button->gameObject.Active = false;
					BoingBoingUI* boing = &button->gameObject.AddComponent<BoingBoingUI>();
					boing->use_on_ui_render2 = true;
					EventListener* eventListener = &button->gameObject.AddComponent<EventListener>();
					eventListener->InvokeOnClickDown();
				}
			}
		}
	}
}

void GameManagerComponent::UpdateOrderSheetAnimation()
{
	using namespace TimeSystem;
	bool remove = false;
	for (int i = 0; i < OrderSheetAnimeVec.size(); i++)
	{
		OrderSheetAnimeData& data = OrderSheetAnimeVec[i];
		if (data.onAnime)
		{
			Transform* tr = data.target;
			data.AnimeStep += Time.DeltaTime;
			float t = data.AnimeStep / data.AnimeTime;
			tr->localPosition = Vector3::Lerp(data.startPos, data.endPos, t);
			if (data.AnimeStep >= data.AnimeTime)
			{
				tr->localPosition = data.endPos;
				data.onAnime = false;
				remove = true;
				OrderSheetAnimeSet.erase(data.target);
			}
		}
	}
	if (remove)
	{
		std::erase_if(OrderSheetAnimeVec, [](OrderSheetAnimeData& data) { return data.onAnime == false; });
	}
}

void GameManagerComponent::UpdateFever()
{
	using namespace TimeSystem;
	if (!hasUiObject[E_UIName::FeverUI])
		return;

	if (IsFever())
	{
		FeverTime += Time.DeltaTime;
		float t = GetFeverTimePercentage();
		FeverFill->uiComponenet.anchorMax.x = (1.f - t);
		if (hasUiObject[E_UIName::FeverVFX])
		{
			for (int i = 0; i < uiArray[E_UIName::FeverVFX].second->transform.GetChildCount(); i++)
			{
				Transform* child = uiArray[E_UIName::FeverVFX].second->transform.GetChild(i);
				((UIMaterialObject&)child->gameObject).uiComponenet.GetCustomData().SetField("t", Mathf::Lerp(0.f, 1.f, t));
			}
		}

		if (FeverTime >= FeverMaxTime)
		{
			FeverEnd();
		}
	}
	else
	{
		float t = GetFeverGaugePercentage();
		FeverFill->uiComponenet.anchorMax.x = t;
	}
	FeverFill->uiComponenet.SetTransform();
}

void GameManagerComponent::OrderSuccessed()
{
	SuccessedOrder++;
	SortOrderSheet();
}

void GameManagerComponent::OrderFailed()
{
	FailedOrder++;
	SortOrderSheet();
}

bool GameManagerComponent::SetOrderSheetAnime(Transform* target, const Vector3& start, const Vector3& end, float time)
{
	auto [iter, result] = OrderSheetAnimeSet.insert(target);
	if (!result)
		return false;

	OrderSheetAnimeData data;
	data.target = target;
	data.AnimeStep = 0.f;
	data.AnimeTime = time;
	data.startPos = start;
	data.endPos = end;
	data.onAnime = true;
	OrderSheetAnimeVec.emplace_back(data);

	return true;
}

void GameManagerComponent::SortOrderSheet()
{
	for (int queueOrder = 0; queueOrder < orderQueue.size(); ++queueOrder)
	{
		int id = orderQueue[queueOrder];
		
		SetOrderSheetAnime(
			OrderSheetVec[id].UI_OrderSheet,
			OrderSheetVec[id].UI_OrderSheet->localPosition,
			OrderSheetPositionX[queueOrder],
			0.3f);
	}
}

int GameManagerComponent::OrderPotionRainbow()
{
	if (VFX_FeverMagic) VFX_FeverMagic->CreateParticle();
	return OrderPotion(PotionType::HealthPotion,
		[this]
		{
			AddScore(RainbowPotion, 20);
		},
		[]{});
}

void GameManagerComponent::SaveGameManagerData() const
{
	using namespace Binary;
	if (!std::filesystem::exists(GameManagerDataPath))
	{
		std::filesystem::create_directories(std::filesystem::path(GameManagerDataPath).parent_path());
	}
	std::ofstream ofs(GameManagerDataPath, std::ios::binary | std::ios::trunc);
	{
		constexpr size_t Version = 0;
		Write::data(ofs, Version);

		for (auto& [path, object] : uiArray)
		{
			Write::wstring(ofs, path);
		}
	}
	ofs.close();
}

void GameManagerComponent::LoadGameManagerData()
{
	using namespace Binary;
	if (!std::filesystem::exists(GameManagerDataPath))
	{
		SaveGameManagerData();
	}	
	std::ifstream ifs(	GameManagerDataPath, std::ios::binary);
	if (ifs.is_open())
	{
		int Version = Read::data<size_t>(ifs);
		for (auto& [path, object] : uiArray)
		{
			path = Read::wstring(ifs);
			if (ifs.eof())
				break;
		}
	}
	ifs.close();

	if (stageNum == 0)
		processTutorialOnStageStart = true;
	else
		processTutorialOnStageStart = false;

	orderQueue.clear();
	int i = -1;
	for (auto& [path, object] : uiArray)
	{
		E_UIName::UIName uiType = static_cast<E_UIName::UIName>(++i);
		if (!path.empty())
		{
			if (object)
			{
				GameObject::Destroy(object);
			}

#ifdef _EDITOR
			if (!Scene::EditorSetting.IsPlay())
				continue;
#endif // _EDITOR

			object = static_cast<UIMaterialObject*>(gameObjectFactory.DeserializedObject(path.c_str()));
			object->Active = false;
			bool isTutorial = stageNum == 0;
			switch (uiType)
			{
			case E_UIName::TimerUI:
			{
				//튜토 예외
				if (isTutorial)
				{
					GameObject::Destroy(object);
					object = nullptr;		
				}
				else
				{
					if (Transform* child = object->transform.GetChild(1))
						TimerClock = static_cast<UIMaterialObject*>(&child->gameObject);
					else
						TimerClock = nullptr;
					if (Transform* child = object->transform.GetChild(2))
						TimerText = child->gameObject.IsComponent<TextRender>();
					else
						TimerText = nullptr;
					if (Transform* child = object->transform.GetChild(3))
						TimerBar = static_cast<UIMaterialObject*>(&child->gameObject);
					else
						TimerBar = nullptr;
				}
			}
			break;
			case E_UIName::ScoreUI:
				//튜토 예외
				if (isTutorial)
				{
					GameObject::Destroy(object);
					object = nullptr;
				}
				else
				{
					if (Transform* child = object->transform.GetChild(3))
						ScoreText = child->gameObject.IsComponent<TextRender>();
					if (Transform* child = object->transform.GetChild(2))
					{
						int childCount = child->GetChildCount();
						UI_Star_FillVec.clear();
						UI_Star_BGVec.clear();
						for (int i = 0; i < childCount; i++)
						{
							if (i < childCount / 2)
							{
								UI_Star_FillVec.push_back((UIMaterialObject*)&child->GetChild(i)->gameObject);
								UI_Star_FillVec[i]->Active = false;
								UI_Star_FillVec[i]->AddComponent<BoingBoingUI>().use_on_ui_render2 = true;
							}
							else
							{
								UI_Star_BGVec.push_back((UIMaterialObject*)&child->GetChild(i)->gameObject);
							}
						}
						UI_Star_BGVec.back()->Active = false;
						UI_Star_FillVec.back()->uiComponenet.Enable = false;
					}
				}			
				break;
			case E_UIName::GameResult:
			{
				Transform* starsParent = object->transform.GetChild(2);//별들
				for (int i = 0; i < starsParent->GetChildCount(); i++)
				{
					if (Transform* star = starsParent->GetChild(i))
					{
						star->gameObject.AddComponent<BoingBoingUI>().use_on_ui_render2 = true;
					}
				}
			}
			break;
			case E_UIName::OderSheetUI:
				object->Active = true;
				OrderSheetVec.clear();
				OrderSheetAnimeSet.clear();
				OrderSheetPositionX.clear();
				OrderSheetVec.reserve(object->transform.GetChildCount());
				OrderSheetAnimeSet.reserve(object->transform.GetChildCount());
				OrderSheetPositionX.reserve(object->transform.GetChildCount());
				for (size_t i = 0; i < object->transform.GetChildCount(); i++)
				{
					OrderSheet oderSheet(&object->transform.GetChild(i)->gameObject);
					oderSheet.SetOrderSheetActiveEmpty();
					oderSheet.UI_OrderSheet->gameObject.Active = false;
					OrderSheetVec.push_back(oderSheet);
					OrderSheetPositionX.push_back(OrderSheetVec[i].UI_OrderSheet->localPosition);
				}
				break;
			case E_UIName::PotionNewsUI:
			{
				if (BackGroundMusicObject* bgmObject = GameObject::Find<BackGroundMusicObject>(L"BackGroundMusic"))
				{
					BGM = bgmObject->component;
				}	
				if (BGM)
				{
					auto temp =
						BGM->eventParametors
						| std::views::filter([](const BankEventParametor& parameter) { return parameter.name == "BGM_Change"; });
					if (!temp.empty())
					{
						temp.front().value = "NewsPaper";
					}
					BGM->Play();
					BGM->SetData("BGM_Change", "NewsPaper");
				}
				object->Active = true;
				potionNewsUI.UI_News = object->transform.GetChild(1);
				GameObject* xButtonObject = &potionNewsUI.UI_News->GetChild(0)->gameObject;
				EventListener& xButtonEventListener = xButtonObject->AddComponent<EventListener>();
				xButtonObject->AddComponent<BoingBoingUI>().use_on_ui_render2 = true;
				xButtonEventListener.SetOnClickUp([this, object, xButtonObject]
					{
						xButtonObject->GetComponent<BoingBoingUI>().Boing(0);
						TimeSystem::Time.DelayedInvok(
							[this, object]
							{
								if (object->Active)
								{
									if (SFX_NewsClick) SFX_NewsClick->Play();
								}
								object->Active = false;

								if (BackGroundMusicObject* bgmObject = GameObject::Find<BackGroundMusicObject>(L"BackGroundMusic"))
								{
									if (BackGroundMusicComponent* BGM = bgmObject->component)
									{
										int stageNumt = std::clamp(stageNum, 1, 3);
										auto temp =
											BGM->eventParametors
											| std::views::filter([](const BankEventParametor& parameter) { return parameter.name == "BGM_Change"; });
										if (!temp.empty())
										{
											temp.front().value = std::format("Stage{}", stageNumt);
										}
										BGM->Play();
										BGM->SetData("BGM_Change", std::format("Stage{}", stageNumt));
									}
								}
							},
							0.3f
						);
					});
				potionNewsUI.NewsTextUI = potionNewsUI.UI_News->GetChild(2);
				potionNewsUI.NewsVec.clear();
				for (int i = 0; i < potionNewsUI.NewsTextUI->GetChildCount(); i++)
				{
					potionNewsUI.NewsVec.push_back((UIMaterialObject*)&potionNewsUI.NewsTextUI->GetChild(i)->gameObject);
					potionNewsUI.NewsVec.back()->Active = false;
				}
				potionNewsUI.StempUI = potionNewsUI.UI_News->GetChild(3);
				potionNewsUI.StempVec.clear();
				for (int i = 0; i < potionNewsUI.StempUI->GetChildCount(); i++)
				{
					potionNewsUI.StempVec.push_back((UIMaterialObject*)&potionNewsUI.StempUI->GetChild(i)->gameObject);
					potionNewsUI.StempVec.back()->Active = false;
				}
				switch (stageNum)
				{
				case 1:
					potionNewsUI.NewsVec[0]->Active = true;
					potionNewsUI.NewsVec[1]->Active = true;
					break;
				case 2:
				case 3:
					if (stageNum == 2)
					{
						potionNewsUI.NewsVec[2]->Active = true;
					}
					else
					{
						potionNewsUI.NewsVec[7]->Active = true;
					}
					if (lastStarCount == 1)
					{
						potionNewsUI.NewsVec[3]->Active = true;
						potionNewsUI.StempVec[0]->Active = true;
					}
					else if (lastStarCount == 2)
					{
						potionNewsUI.NewsVec[4]->Active = true;
						potionNewsUI.StempVec[1]->Active = true;
					}
					else if (lastStarCount == 3)
					{
						potionNewsUI.NewsVec[5]->Active = true;
						potionNewsUI.StempVec[2]->Active = true;
					}
					else if (lastStarCount == 4)
					{
						potionNewsUI.NewsVec[6]->Active = true;
						potionNewsUI.StempVec[3]->Active = true;
					}
					else
					{
						//재시작시 신문 X
						object->Active = false;
					}
					break;
				default:
					object->Active = false;
					break;
				}
			}
			break;
			case E_UIName::FeverUI:
				if (isTutorial)
				{
					GameObject::Destroy(object);
					object = nullptr;
				}
				else
				{
					FeverFill = (UIMaterialObject*)&object->transform.GetChild(0)->gameObject;
					FeverFill->uiComponenet.anchorMax.x = 0.f;
					FeverFill->uiComponenet.SetTransform();
				}
				break;
			case E_UIName::FeverVFX:
				

				break;
			default:
				break;
			}
		}
	}


	//auto gameStartUI = GameObject::Find(L"GameStartUI");
	//if (gameStartUI)
	//{
	//	gameStartUI->AddComponent<GameStartUIComponent>().isReverse = true;
	//	gameStartUI->Active = true;
	//}
	//auto gameStartUI2 = GameObject::Find(L"GameStartUI2");
	//if (gameStartUI2)
	//{
	//	gameStartUI2->Active = true;
	//}

}

void GameManagerComponent::SavePotionScoreData() const
{
	using namespace Binary;
	if (!std::filesystem::exists(PotionScoreDataPath))
	{
		std::filesystem::create_directories(std::filesystem::path(PotionScoreDataPath).parent_path());
	}
	std::ofstream ofs(PotionScoreDataPath, std::ios::binary | std::ios::trunc);
	{
		constexpr size_t Version = 1;
		Write::data(ofs, Version);

		for (auto& i : potionScore)
		{
			Write::data(ofs, i);
		}
	}
	ofs.close();
}

void GameManagerComponent::LoadPotionScoreData()
{
	using namespace Binary;
	std::ifstream ifs(PotionScoreDataPath, std::ios::binary);
	if (ifs.is_open())
	{
		size_t Version = 0;
		Version = Read::data<size_t>(ifs);

		if (Version == 0)
		{
			for (int i = 0; i < 6; i++)
			{
				if (ifs.eof())
					break;
				potionScore[i] = Read::data<int>(ifs);
			}
		}
		if (Version > 0)
		{
			for (auto& item : potionScore)
			{
				item = Read::data<int>(ifs);
			}
		}
	}
	ifs.close();
}


constexpr const wchar_t* StageDataPath[]
{
	 L"Resource/GameManagerData/StageData/Stage1.BinaryData",
	 L"Resource/GameManagerData/StageData/Stage2.BinaryData",
	 L"Resource/GameManagerData/StageData/Stage3.BinaryData"
};
void GameManagerComponent::SaveStageData() const
{
	using namespace Binary;
	if (std::size(StageDataPath) < stageNum || stageNum < 1)
		return;

	std::filesystem::path dataPath =  StageDataPath[stageNum - 1];
	if (!std::filesystem::exists(dataPath))
	{
		std::filesystem::create_directories(dataPath.parent_path());
	}
	else
	{
		bool ok = WinUtility::ShowConfirmationDialog(L"기존에 있던 내용은 덮어쓰기 됩니다.", L"저장하시겠습니까?");
		if (!ok)
		{
			return;
		}
	}
	std::ofstream ofs(dataPath, std::ios::binary | std::ios::trunc);
	{
		constexpr size_t Version = 4;
		Write::data(ofs, Version);

		Write::data(ofs, PlayTime);
		for (auto& item : starScores)
		{
			Write::data(ofs, item);
		}

		if constexpr (Version > 0)
		{
			Write::data(ofs, FeverMaxPercent);
			Write::data(ofs, FeverPercent);
			Write::data(ofs, FeverMaxTime);
		}

		if constexpr (Version > 1)
		{
			Write::data(ofs, trophyScore);
		}

		if constexpr (Version > 2)
		{
			Write::string(ofs, coin);
		}

		if constexpr (Version > 3)
		{
			Write::string(ofs, fever_bubble);
			Write::string(ofs, fever_magic);
			Write::string(ofs, lightening);
			Write::string(ofs, during_fever);
		}
	}
	ofs.close();
}

void GameManagerComponent::LoadStageData()
{
	using namespace Binary;
	if (std::size(StageDataPath) < stageNum || stageNum < 1)
		return;

	std::filesystem::path dataPath = StageDataPath[stageNum - 1];
	std::ifstream ifs(dataPath, std::ios::binary);
	if (ifs.is_open())
	{
		size_t Version = 0;
		Version = Read::data<size_t>(ifs);

		PlayTime = Read::data<float>(ifs);
		for (auto& item : starScores)
		{
			item = Read::data<int>(ifs);
		}

		if (Version > 0)
		{
			FeverMaxPercent = Read::data<float>(ifs);
			FeverPercent = Read::data<float>(ifs);
			FeverMaxTime = Read::data<float>(ifs);
		}

		if (Version > 1)
		{
			trophyScore = Read::data<int>(ifs);
		}

		if (Version > 2)
		{
			coin = Read::string(ifs);
		}
		if (Version > 3)
		{
			fever_bubble = Read::string(ifs);
			fever_magic = Read::string(ifs);
			lightening = Read::string(ifs);
			during_fever = Read::string(ifs);
		}
	}
	ifs.close();
}

constexpr const wchar_t* PotionWeightPath = L"Resource/GameManagerData/PotionWeightPath.BinaryData";
void GameManagerComponent::SavePotionWeightData()
{
	std::filesystem::path dataPath(PotionWeightPath);
	if (!std::filesystem::exists(dataPath))
	{
		std::filesystem::create_directories(dataPath.parent_path());
	}

	std::ofstream ofs(dataPath, std::ios::binary | std::ios::trunc);
	{
		Binary::Write::data<int>(ofs, potionWeight0.first);
		Binary::Write::data<int>(ofs, potionWeight0.second);

		//1스테
		for (auto& item : potionWeight1)
		{
			Binary::Write::data<int>(ofs, item.first);
			Binary::Write::data<int>(ofs, item.second);
		}
		//2스테
		for (auto& item : potionWeight2)
		{
			Binary::Write::data<int>(ofs, item.first);
			Binary::Write::data<int>(ofs, item.second);
		}
		//3스테
		for (auto& item : potionWeight3)
		{
			Binary::Write::data<int>(ofs, item.first);
			Binary::Write::data<int>(ofs, item.second);
		}
	}
	ofs.close();
}

void GameManagerComponent::LoadPotionWeightData()
{
	std::filesystem::path dataPath(PotionWeightPath);
	std::ifstream ifs(dataPath, std::ios::binary);
	if(ifs.is_open())
	{
		potionWeight0.first = (PotionType)Binary::Read::data<int>(ifs);
		potionWeight0.second = Binary::Read::data<int>(ifs);
		
		//1스테
		for (auto& item : potionWeight1)
		{
			item.first  = (PotionType)Binary::Read::data<int>(ifs);
			item.second = Binary::Read::data<int>(ifs);
		}
		//2스테
		for (auto& item : potionWeight2)
		{
			item.first = (PotionType)Binary::Read::data<int>(ifs);
			item.second = Binary::Read::data<int>(ifs);
		}
		//3스테
		for (auto& item : potionWeight3)
		{
			item.first = (PotionType)Binary::Read::data<int>(ifs);
			item.second = Binary::Read::data<int>(ifs);
		}
	}
	ifs.close();
}

int GameManagerComponent::OrderPotion(PotionType potionType, const std::function<void()>& SuccessedCallBack, const std::function<void()>& FaildCallBack)
{
	if (!IsStageStart() || IsTimeEnd() || orderQueue.size() == 5)
		return -1;

	for (int i = 0; i < OrderSheetVec.size(); ++i)
	{
		if (OrderSheetVec[i].UI_OrderSheet->gameObject.Active == false)
		{
			OrderSheetVec[i].UI_OrderSheet->gameObject.Active = true;
			OrderSheetVec[i].SetOrderSheet(potionType);
			OrderSheetVec[i].successedCallBack = SuccessedCallBack;
			OrderSheetVec[i].faildCallBack = FaildCallBack;
			Debug_printf("주문 : %s\n", GetPotionName(potionType));

			OrderSheetVec[i].UI_OrderSheet->localPosition = OrderSheetPositionX[orderQueue.size()];
			SetOrderSheetAnime(
				OrderSheetVec[i].UI_OrderSheet,
				OrderSheetVec[i].UI_OrderSheet->localPosition + Vector3::Down * 0.5f,
				OrderSheetVec[i].UI_OrderSheet->localPosition,
				.3f
			);
			if (!IsFever())
			{
				Vector3 start = OrderSheetVec[i].StuffPaperUI->localPosition + Vector3::Down * 0.5f;
				Vector3 end   = OrderSheetVec[i].StuffPaperUI->localPosition;
				TimeSystem::Time.DelayedInvok(
					[this, target = OrderSheetVec[i].StuffPaperUI, start, end]()
					{
						SetOrderSheetAnime(target, start, end, 0.3f);
					},
					0.2f);
				OrderSheetVec[i].StuffPaperUI->localPosition = start;
			}
			else
			{
				Transform* PotionIcon = OrderSheetVec[i].PotionIcon;
				switch (OrderSheetVec[i].GetPotionType())
				{
				case HealthPotion:
					PotionIcon->GetChild(0)->gameObject.Active = false;
					break;
				case ManaPotion:
					PotionIcon->GetChild(1)->gameObject.Active = false;
					break;
				case AddictionPotion:
					PotionIcon->GetChild(2)->gameObject.Active = false;
					break;
				case SeductionPotion:
					PotionIcon->GetChild(3)->gameObject.Active = false;
					break;
				case FlamePotion:
					PotionIcon->GetChild(4)->gameObject.Active = false;
					break;
				case FrozenPotion:
					PotionIcon->GetChild(5)->gameObject.Active = false;
					break;
				default:
					break;
				}
				PotionIcon->GetChild(6)->gameObject.Active = true;
			}
			orderQueue.push_back(i);
			if (SFX_Order) SFX_Order->Play();
			return i;
		}
	}
	Debug_printf("주문서 꽉참!!!\n");
	return -1;
}

void GameManagerComponent::OrderSheetUpdate(int id, float t)
{
	if (!IsStageStart() || IsTimeEnd())
		return;

	if (OrderSheetVec[id].UI_OrderSheet->gameObject.Active == true)
	{
		OrderSheetVec[id].SetPatienceFill(t);
		if (t <= 0.f)
		{
			OrderSheetVec[id].UI_OrderSheet->gameObject.Active = false;
			std::erase(orderQueue, id);
			Debug_printf("응 안살꺼야\n");
			OrderSheetVec[id].faildCallBack();
			OrderFailed();
		}
		else if (t < 0.3f)
		{		
			OrderSheetVec[id].UI_Potion_Paper->uiComponenet.PositionAnimation(Vector2(0.001f, 0.f), 0.1f);
			OrderSheetVec[id].UI_Stuff_PaperLeft->uiComponenet.PositionAnimation(Vector2(0.001f, 0.f), 0.1f);
			OrderSheetVec[id].UI_Stuff_PaperRight->uiComponenet.PositionAnimation(Vector2(0.001f, 0.f), 0.1f);
		}
	}
}

int GameManagerComponent::ServeOrder(PotionType potionType)
{
	if (!IsStageStart() || IsTimeEnd())
		return -1;

	int target_index = -1;
	if ((potionType & PotionType::RainbowPotion) != 0)
	{
		if (!IsFever() || orderQueue.empty())
		{
			Debug_printf("주문서에 그런건 없어.\n");
			return target_index;
		}
		else
		{
			int index = orderQueue.front();
			OrderSheetVec[index].UI_OrderSheet->gameObject.Active = false;
			OrderSheetVec[index].successedCallBack();
			orderQueue.erase(orderQueue.begin());
			Debug_printf("%s 처리\n", GetPotionName(potionType));
			OrderSuccessed();
			TimeSystem::Time.DelayedInvok(
				[this] {
					OrderPotionRainbow();
				}, 0.f);
			return index;
		}
	}


	std::vector<int> _orderQueueCopy = orderQueue;
	for (int i = 0; i < _orderQueueCopy.size(); ++i)
	{
		int _orderIdx = _orderQueueCopy[i];
		if (OrderSheetVec[_orderIdx].GetPotionType() == potionType)
		{
			OrderSheetVec[_orderIdx].UI_OrderSheet->gameObject.Active = false;
			OrderSheetVec[_orderIdx].successedCallBack();

			target_index = i;
			break;
		}
	}
	if (target_index > -1)
	{
		orderQueue.erase(orderQueue.begin() + target_index);
		Debug_printf("%s 처리\n", GetPotionName(potionType));
		OrderSuccessed();
	}
	else
		Debug_printf("주문서에 그런건 없어.\n");

	return target_index;
}

void GameManagerComponent::OrderDeleteFront()
{
	int id = orderQueue.front();
	if (OrderSheetVec[id].UI_OrderSheet->gameObject.Active == true)
	{
		if (!IsStageStart() || IsTimeEnd())
			return;

		OrderSheetVec[id].UI_OrderSheet->gameObject.Active = false;
		std::erase(orderQueue, id);
		SortOrderSheet();
	}
}

int GameManagerComponent::GetPotionScore(PotionType potionType) const
{
	switch (potionType)
	{
	case HealthPotion:
		return potionScore[0];
	case AddictionPotion:	   
		return potionScore[1];
	case FlamePotion:		   
		return potionScore[2];
	case SeductionPotion:	   
		return potionScore[3];
	case ManaPotion:		   
		return potionScore[4];
	case FrozenPotion:		   
		return potionScore[5];
	case RainbowPotion:
		return potionScore[6];
	default:
		return 0;
	}
}

int GameManagerComponent::AddScore(PotionType potionType, int _tip)
{
	int score = GetPotionScore(potionType);
	return AddScore(score, _tip);
}

int GameManagerComponent::AddScore(int score, int _tip)
{
	using namespace E_UIName;
	Score += score;
	Score += _tip;
	tip	  += _tip;
	if (hasUiObject[ScoreUI] && ScoreText)
	{
		ScoreText->Highlight(1.4f, Color(0, 1, 0), 1.f);
	}

	AddFeverPercent();
	if (!isFever && SFX_Gold) SFX_Gold->Play();
	if (isFever && SFX_Gold_Pang) SFX_Gold_Pang->Play();
	if (isFever && VFX_Coin) VFX_Coin->CreateParticle();
	return Score;
}

int GameManagerComponent::GetTotalScore() const
{
	return Score;
}

int GameManagerComponent::GetScore() const
{
	return Score - tip;
}

int GameManagerComponent::GetTip() const
{
	return tip;
}

void GameManagerComponent::UnlockSecret()
{
	if (UIMaterialObject* award = UI_Star_FillVec[3])
	{
		award->Active = true;
	}
}

int GameManagerComponent::GetStarCount()
{
	if (hasUiObject[E_UIName::ScoreUI] == false)
		return 0;

	int count = 0;
	for (int i = 0; i < UI_Star_FillVec.size(); i++)
	{
		UIMaterialObject* uiStar = UI_Star_FillVec[i];
		if (uiStar && uiStar->Active)
		{
			if(i != 3)
				++count;
			else if (UI_Star_FillVec[2]->Active)
				++count;
		}
	}
	return count;
}

void GameManagerComponent::StageLoad(int _stageNum)
{
	//스테이지 씬 파일 이름 정해야함
	constexpr const wchar_t* stageFileTable[] =
	{
	  L"EngineResource/NewSampleScene.Scene",
	  L"EngineResource/NewSampleScene.Scene",
	  L"EngineResource/NewSampleScene.Scene",
	  L"EngineResource/NewSampleScene.Scene",
	};

	if (std::size(stageFileTable) <= _stageNum || _stageNum < 0)
	{
		SQLiteLogger::GameLog("Warning", "GameManager Warning : StageLoad(int) stageNum is out of index");
		return;
	}

	auto temp = GameObject::FindFirst<GameManagerHelper>();
	if (temp)
	{
		helper = GameObject::FindFirst<GameManagerHelper>()->component;
	}
	std::filesystem::path stagePath = stageFileTable[_stageNum];
	if(helper)
	{
		stagePath = helper->stagePaths[_stageNum];
	}
	if(std::filesystem::exists(stagePath))
	{
		sceneManager.LoadScene(stagePath.c_str());

		Score = 0;
		tip = 0;
		CurrentFeverPercent = 0.f;
		FeverTime = 0.0f;
		ResetFlags();
		stageNum = _stageNum;
		isinit = false;
		for (auto& [path, object] : uiArray)
		{
			object = nullptr;
		}
		memset(uiElapsedTimes, 0.f, sizeof(uiElapsedTimes));
		LoadGameManagerData();
		LoadStageData();
		TimeSystem::Time.UpdateTime();
	}
}

void GameManagerComponent::StageClear()
{
	isStageClear = true;
}

void GameManagerComponent::MainMenuScene()
{
	std::wstring wstr = L"EngineResource/MainMenu/MainMenu.Scene";
	if (helper && helper->mainMenuPath.size())
	{
		wstr.assign(helper->mainMenuPath.begin(), helper->mainMenuPath.end());
	}
	sceneManager.LoadScene(wstr.c_str());
	ClearUIObjects();
	GameObject::Destroy(gameObject);
}

void GameManagerComponent::EndingScene()
{

	std::wstring wstr = L"TT.Scene";
	if (helper && helper->mainMenuPath.size())
	{
		wstr.assign(helper->gameOverScenePath.begin(), helper->gameOverScenePath.end());
	}
	sceneManager.LoadScene(wstr.c_str());
	ClearUIObjects();
	GameObject::Destroy(gameObject);
}

void GameManagerComponent::GameOverScene()
{
	std::wstring wstr = L"EngineResource/GameOverScene.Scene";
	if (helper && helper->mainMenuPath.size())
	{
		wstr.assign(helper->gameOverScenePath.begin(), helper->gameOverScenePath.end());
	}
	sceneManager.LoadScene(wstr.c_str());
	ClearUIObjects();
	GameObject::Destroy(gameObject);
}

GameManagerComponent::OrderSheet::OrderSheet(GameObject* rootOrderSheet)
{
	UI_OrderSheet = &rootOrderSheet->transform;
	UI_Potion_Paper = (UIMaterialObject*)&UI_OrderSheet->GetChild(0)->gameObject;
	UI_Patience_Bar_Fill = (UIMaterialObject*)&UI_OrderSheet->GetChild(1)->GetChild(0)->gameObject;
	UI_Patience_Bar_Fill->uiComponenet.GetCustomData().SetField("hue", 0.333f);
	PotionIcon = UI_OrderSheet->GetChild(3);


	StuffPaperUI = UI_OrderSheet->GetChild(2);

	StuffPaperUILeft = StuffPaperUI->GetChild(0);
	UI_Stuff_PaperLeft = (UIMaterialObject*)&StuffPaperUILeft->GetChild(0)->gameObject;
	ItemIconLeft = StuffPaperUILeft->GetChild(2);
	QTEIconLeft  = StuffPaperUILeft->GetChild(3);

	StuffPaperUIRight = StuffPaperUI->GetChild(1);
	UI_Stuff_PaperRight = (UIMaterialObject*)&StuffPaperUIRight->GetChild(0)->gameObject;
	ItemIconRight = StuffPaperUIRight->GetChild(2);
	QTEIconRight  = StuffPaperUIRight->GetChild(3);
}

void GameManagerComponent::OrderSheet::SetOrderSheetActiveEmpty()
{
	for (size_t i = 0; i < PotionIcon->GetChildCount(); i++)
	{
		UIMaterialObject* icon = (UIMaterialObject*)&PotionIcon->GetChild(i)->gameObject;
		icon->Active = false;	
	}
	for (size_t i = 0; i < ItemIconLeft->GetChildCount(); i++)
	{
		UIMaterialObject* icon = (UIMaterialObject*)&ItemIconLeft->GetChild(i)->gameObject;
		icon->Active = false;
	}	
	for (size_t i = 0; i < QTEIconLeft->GetChildCount(); i++)
	{
		UIMaterialObject* icon = (UIMaterialObject*)&QTEIconLeft->GetChild(i)->gameObject;
		icon->Active = false;
	}
	for (size_t i = 0; i < ItemIconRight->GetChildCount(); i++)
	{
		UIMaterialObject* icon = (UIMaterialObject*)&ItemIconRight->GetChild(i)->gameObject;
		icon->Active = false;
	}
	for (size_t i = 0; i < QTEIconRight->GetChildCount(); i++)
	{
		UIMaterialObject* icon = (UIMaterialObject*)&QTEIconRight->GetChild(i)->gameObject;
		icon->Active = false;
	}
}

void GameManagerComponent::OrderSheet::SetOrderSheet(PotionType potionType)
{
	static std::vector<IngredientType> recipe; //재할당 방지 static
	recipe = RecipeManager::GetInstance()->GetIngredients(potionType);
	SetOrderSheetActiveEmpty();
	this->potionType = potionType;
	UI_Patience_Bar_Fill->uiComponenet.GetCustomData().SetField("hue", 0.333f);
	switch (potionType)
	{
	case HealthPotion:
		PotionIcon->GetChild(0)->gameObject.Active = true;
		break;
	case ManaPotion:
		PotionIcon->GetChild(1)->gameObject.Active = true;
		break;
	case AddictionPotion:
		PotionIcon->GetChild(2)->gameObject.Active = true;
		break;
	case SeductionPotion:
		PotionIcon->GetChild(3)->gameObject.Active = true;
		break;
	case FlamePotion:
		PotionIcon->GetChild(4)->gameObject.Active = true;
		break;
	case FrozenPotion:
		PotionIcon->GetChild(5)->gameObject.Active = true;
		break;
	case RainbowPotion:
		PotionIcon->GetChild(6)->gameObject.Active = true;
		break;
	default:
		break;
	}

	constexpr float StuffPaperUIMoveDampY = 0.08f;
	if(!recipe.empty())
	{
		StuffPaperUILeft->gameObject.Active = true;
		IngredientType& leftType = recipe[0];
		if (leftType <= MagicWood)
		{
			switch (leftType)
			{
			case IngredientType::DragonTail:
				ItemIconLeft->GetChild(0)->gameObject.Active = true;
				break;
			case IngredientType::MagicFlower:
				ItemIconLeft->GetChild(1)->gameObject.Active = true;
				break;
			case IngredientType::MagicWood:
				ItemIconLeft->GetChild(2)->gameObject.Active = true;
				break;
			default:
				break;
			}
				
			if (moveDownLeft)
			{
				UI_Stuff_PaperLeft->uiComponenet.MoveY(StuffPaperUIMoveDampY);
				moveDownLeft = false;
			}
		}
		else
		{
			switch (leftType)
			{
			case IngredientType::SlicedDragonTail:
			case IngredientType::CompressedDragonTail:
				ItemIconLeft->GetChild(0)->gameObject.Active = true;
				break;
			case IngredientType::GrindedMagicFlower:
			case IngredientType::CompressedMagicFlower:
				ItemIconLeft->GetChild(1)->gameObject.Active = true;
				break;
			case IngredientType::SlicedMagicWood:
			case IngredientType::GrindedMagicWood:
				ItemIconLeft->GetChild(2)->gameObject.Active = true;
				break;
			default:
				break;
			}
			switch (leftType)
			{
			case IngredientType::SlicedMagicWood:
			case IngredientType::SlicedDragonTail:
				QTEIconLeft->GetChild(0)->gameObject.Active = true;
				break;
			case IngredientType::GrindedMagicFlower:
			case IngredientType::GrindedMagicWood:
				QTEIconLeft->GetChild(1)->gameObject.Active = true;
				break;
			case IngredientType::CompressedDragonTail:
			case IngredientType::CompressedMagicFlower:
				QTEIconLeft->GetChild(2)->gameObject.Active = true;
				break;
			default:
				break;
			}

			if (!moveDownLeft)
			{
				UI_Stuff_PaperLeft->uiComponenet.MoveY(-StuffPaperUIMoveDampY);
				moveDownLeft = true;
			}
		}
	}
	else
	{
		__debugbreak();
	}
	if(2 <= recipe.size())
	{
		StuffPaperUIRight->gameObject.Active = true;
		IngredientType& rightType = recipe[1];
		if (rightType <= MagicWood)
		{
			switch (rightType)
			{
			case IngredientType::DragonTail:
				ItemIconRight->GetChild(0)->gameObject.Active = true;
				break;
			case IngredientType::MagicFlower:
				ItemIconRight->GetChild(1)->gameObject.Active = true;
				break;
			case IngredientType::MagicWood:
				ItemIconRight->GetChild(2)->gameObject.Active = true;
				break;
			default:
				break;
			}

			if (moveDownRight)
			{
				UI_Stuff_PaperRight->uiComponenet.MoveY(StuffPaperUIMoveDampY);
				moveDownRight = false;
			}
		}
		else
		{
			switch (rightType)
			{
			case IngredientType::SlicedDragonTail:
			case IngredientType::CompressedDragonTail:
				ItemIconRight->GetChild(0)->gameObject.Active = true;
				break;
			case IngredientType::GrindedMagicFlower:
			case IngredientType::CompressedMagicFlower:
				ItemIconRight->GetChild(1)->gameObject.Active = true;
				break;
			case IngredientType::SlicedMagicWood:
			case IngredientType::GrindedMagicWood:
				ItemIconRight->GetChild(2)->gameObject.Active = true;
				break;
			default:
				break;
			}
			switch (rightType)
			{
			case IngredientType::SlicedMagicWood:
			case IngredientType::SlicedDragonTail:
				QTEIconRight->GetChild(0)->gameObject.Active = true;
				break;
			case IngredientType::GrindedMagicFlower:
			case IngredientType::GrindedMagicWood:
				QTEIconRight->GetChild(1)->gameObject.Active = true;
				break;
			case IngredientType::CompressedDragonTail:
			case IngredientType::CompressedMagicFlower:
				QTEIconRight->GetChild(2)->gameObject.Active = true;
				break;
			default:
				break;
			}

			if (!moveDownRight)
			{
				UI_Stuff_PaperRight->uiComponenet.MoveY(-StuffPaperUIMoveDampY);
				moveDownRight = true;
			}
		}
	}
	else
	{
		StuffPaperUIRight->gameObject.Active = false;
	}
}

void GameManagerComponent::OrderSheet::SetPatienceFill(float t)
{
	UI_Patience_Bar_Fill->uiComponenet.anchorMax.x = Mathf::Lerp(0.f, 1.f, t);
	UI_Patience_Bar_Fill->uiComponenet.GetCustomData().SetField("hue", Mathf::Lerp(0.f, 0.333f, t));
	UI_Patience_Bar_Fill->uiComponenet.SetTransform();
}

void GameManagerHelperComponent::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, 1);

	for (auto& item : stagePaths)
	{
		Binary::Write::string(ofs, item);
	}
	Binary::Write::string(ofs, endingScenePath);
	Binary::Write::string(ofs, gameOverScenePath);
	Binary::Write::string(ofs, mainMenuPath);

}

void GameManagerHelperComponent::Deserialized(std::ifstream& ifs)
{
	int version = Binary::Read::data<int>(ifs);

	for (auto& item : stagePaths)
	{
		item = Binary::Read::string(ifs);
	}
	endingScenePath = Binary::Read::string(ifs);
	gameOverScenePath = Binary::Read::string(ifs);
	mainMenuPath = Binary::Read::string(ifs);
}

void GameManagerHelperComponent::InspectorImguiDraw()
{

	for (int i = 0; i < std::size(stagePaths); i++)
	{
		ImGui::InputText(std::format("Stage Path {}", i).c_str(), (char*)stagePaths[i].c_str(), stagePaths[i].size(), ImGuiInputTextFlags_CallbackResize,
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
						 }, & stagePaths[i]);
	}

	ImGui::InputText("Ending Scene Path", (char*)endingScenePath.c_str(), endingScenePath.size(), ImGuiInputTextFlags_CallbackResize,
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
					 }, &endingScenePath);

	ImGui::InputText("Game Over Scene Path", (char*)gameOverScenePath.c_str(), gameOverScenePath.size(), ImGuiInputTextFlags_CallbackResize,
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
					 }, &gameOverScenePath);

	ImGui::InputText("Main Menu Path", (char*)mainMenuPath.c_str(), mainMenuPath.size(), ImGuiInputTextFlags_CallbackResize,
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
					 }, &mainMenuPath);



};
