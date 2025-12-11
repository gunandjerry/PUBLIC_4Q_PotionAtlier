#pragma once
#include "Components/Cauldron.h"
#include <vector>
#include <functional>

class TextRender;
class UIMaterialObject;
namespace E_UIName
{
	enum UIName
	{
		//길게 눌러 영업 시작
		StageStartUI,
		//날짜 간판 (첫째날)
		StageUI,
		//준비됐나요?
		Ready,
		//영업 시작!
		Start,
		//영업 종료!
		End,
		//영업 결과 UI
		GameResult,
		//타이머 UI
		TimerUI,
		//점수 UI
		ScoreUI,
		//주문서 UI
		OderSheetUI,
		//신문 UI
		PotionNewsUI,
		//피버 UI
		FeverUI,
		//피버 VFX,
		FeverVFX,
		Null,
	};

	inline const char* GetUINameToString(UIName value)
	{
		static const char8_t* array[UIName::Null]
		{
			u8"길게 눌러 영업 시작",
			u8"스테이지 간판",
			u8"준비됐나요?",
			u8"영업 시작!",
			u8"영업 종료!",
			u8"영업 결과 UI",
			u8"타이머 UI",
			u8"점수 UI",
			u8"주문서 UI",
			u8"신문 UI",
			u8"피버 UI",
			u8"피버 VFX"
		};
		return (const char*)array[value];
	}
}
class GameManagerHelperComponent : public Component
{
public:
	virtual void Awake() {}
	virtual void Start() {}
	std::string stagePaths[4]{};
	std::string endingScenePath{};
	std::string mainMenuPath{};
	std::string gameOverScenePath{};
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);
	virtual void InspectorImguiDraw();

protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}

};
class GameManagerComponent : public Component
{
public:
	inline static constexpr const wchar_t* GameManagerDataPath = L"Resource/GameManagerData/GameManager.BinaryData";
	inline static constexpr const wchar_t* PotionScoreDataPath = L"Resource/GameManagerData/PotionScore.BinaryData";

	GameManagerComponent();
	virtual ~GameManagerComponent() override;
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Serialized(std::ofstream& ofs) override {}
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Deserialized(std::ifstream& ifs) override {}
	virtual void InspectorImguiDraw() override;
public:
	virtual void Awake() override;
	virtual void Start() override;
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override;
	virtual void LateUpdate() override {}

public:
	/*제한 시간*/
	float PlayTime = 120.f;
	/*별 목표 점수*/
	int starScores[3] = { 10, 20, 30 };
	/*트로피 목표 점수*/
	int trophyScore = { 40 };

	/* 카운터 펀치 딜레이 */
	float counter_punch_elapsed_time{ 0.0f };
	float counter_punch_cooltime{ 25.0f };
	float counter_punch_cooltime_max{ 30.0f };
	float counter_punch_cooltime_min{ 20.0f };


private:
	/*현재 총 점수*/
	int Score = 0;
	/*현재 팁*/
	int tip = 0;
	/*처리한 주문*/
	int SuccessedOrder = 0;
	int FailedOrder = 0;

	int stageNum = 0;

	/*마지막 점수*/
	int lastScore = 0;
	/*마지막 별 개수*/
	int lastStarCount = 0;
	bool isStageClear = false;
	bool isStageStart = false;
	bool isTimeEnd = false;

	// 튜토리얼
	bool processTutorialOnStageStart{ false };

	inline static constexpr size_t UiArrayCount = E_UIName::Null;
	float uiElapsedTimes[UiArrayCount]{};
	std::pair<std::wstring, UIMaterialObject*> uiArray[UiArrayCount];
	bool hasUiObject[UiArrayCount]{};

	std::vector<UIMaterialObject*> UI_OrderSheetVec;
	std::vector<Vector3> UI_OrderSheetStartPosVec;
	std::vector<UIMaterialObject*> UI_PatienceBarVec;
	std::vector<GameObject*>	   StuffPaperUIVec;

	//컴포넌트 캐싱용
	TextRender* TimerText = nullptr;
	UIMaterialObject* TimerBar = nullptr;
	UIMaterialObject* TimerClock = nullptr; 
	GameManagerHelperComponent* helper = nullptr;

	class ParticleSpawnComponent* VFX_Coin{ nullptr };
	class ParticleSpawnComponent* VFX_FeverBubble{ nullptr };
	class ParticleSpawnComponent* VFX_FeverMagic{ nullptr };
	class ParticleSpawnComponent* VFX_Lightening{ nullptr };
	class ParticleSpawnComponent* VFX_DuringFever{ nullptr };
	class AudioBankClip* BGM{ nullptr };
	class AudioBankClip* SFX_NewsClick{ nullptr };
	class AudioBankClip* SFX_Order{ nullptr };
	class AudioBankClip* SFX_Gold{ nullptr };
	class AudioBankClip* SFX_Gold_Pang{ nullptr };
	class AudioBankClip* SFX_Fever{ nullptr };
	class AudioBankClip* SFX_Result_Popup{ nullptr };
	class AudioBankClip* SFX_Result_Star1{ nullptr };
	class AudioBankClip* SFX_Result_Star2{ nullptr };
	class AudioBankClip* SFX_Result_Star3{ nullptr };
	class AudioBankClip* SFX_Trophy{ nullptr };
	class AudioBankClip* SFX_ClassA{ nullptr };
	class AudioBankClip* SFX_ClassB{ nullptr };
	class AudioBankClip* SFX_ClassC{ nullptr };
	class AudioBankClip* SFX_Hidden{ nullptr };
	class AudioBankClip* SFX_Click{ nullptr };
	class AudioBankClip* SFX_Ready{ nullptr };
	class AudioBankClip* SFX_Go{ nullptr };

	std::string coin{};
	std::string fever_bubble{};
	std::string fever_magic{};
	std::string lightening{};
	std::string during_fever{};

	TextRender* ScoreText = nullptr;
	std::vector<UIMaterialObject*> UI_Star_FillVec;
	std::vector<UIMaterialObject*> UI_Star_BGVec;

	bool isinit = false;
	bool isinit1Tick = false;
	//주문서
	struct OrderSheet
	{
		OrderSheet(GameObject* rootOrderSheet);
		void SetOrderSheetActiveEmpty();
		void SetOrderSheet(PotionType potionType);
		void SetPatienceFill(float t);
		PotionType GetPotionType() const { return potionType; }

		Transform* UI_OrderSheet = nullptr;
		UIMaterialObject* UI_Potion_Paper = nullptr;
		UIMaterialObject* UI_Patience_Bar_Fill = nullptr;
		Transform* PotionIcon = nullptr;

		Transform* StuffPaperUI = nullptr;
		Transform* StuffPaperUILeft = nullptr;
		UIMaterialObject* UI_Stuff_PaperLeft = nullptr;
		Transform* ItemIconLeft = nullptr;
		Transform* QTEIconLeft = nullptr;

		Transform* StuffPaperUIRight = nullptr;
		UIMaterialObject* UI_Stuff_PaperRight = nullptr;
		Transform* ItemIconRight = nullptr;
		Transform* QTEIconRight = nullptr;

		std::function<void()> successedCallBack;
		std::function<void()> faildCallBack;
	private:
		PotionType potionType = PotionType::FailurePotion;
		bool moveDownLeft = false;
		bool moveDownRight = false;
	};
	std::vector<OrderSheet> OrderSheetVec;
	std::vector<Vector3> OrderSheetPositionX;
	struct OrderSheetAnimeData
	{
		Transform* target = nullptr;
		bool onAnime = false;;
		Vector3 startPos;
		Vector3 endPos;
		float AnimeStep = 0;
		float AnimeTime = 0;
	};
	std::vector<OrderSheetAnimeData> OrderSheetAnimeVec;
	std::unordered_set<Transform*>   OrderSheetAnimeSet;

	std::vector<int> orderQueue;
	struct 
	{
		Transform* UI_News = nullptr;

		Transform* NewsTextUI = nullptr;
		std::vector<UIMaterialObject*> NewsVec;
		Transform* StempUI = nullptr;
		std::vector<UIMaterialObject*> StempVec;
	}
	potionNewsUI;


public:
	void AddFeverPercent();
	void FeverStart();
	void FeverEnd();

	bool IsFever() const { return isFever; }
	float GetFeverGaugePercentage() const { return CurrentFeverPercent / FeverMaxPercent; }
	float GetFeverTimePercentage() const { return FeverTime / FeverMaxTime; }

	std::vector<std::pair<PotionType, int>> GetCurrentPotionWeight() const;
private:
	/*현재 게이지*/
	float CurrentFeverPercent = 0.f;
	/*최대 게이지*/
	float FeverMaxPercent = 25.f;
	/*주문 처리당 차는 게이지*/
	float FeverPercent = 5.f;

	/*피버 진행 시간*/
	float FeverTime = 0.0f;
	/*피버 지속 시간*/
	float FeverMaxTime = 10.0f;
	bool isFever = false;

	UIMaterialObject* FeverFill = nullptr;

	//스테이지별 포션 가중치
    std::pair<PotionType, int> potionWeight0;
	std::pair<PotionType, int> potionWeight1[2];
	std::pair<PotionType, int> potionWeight2[4];
	std::pair<PotionType, int> potionWeight3[6];
private:
	void ClearUIObjects();
	void ResetFlags();

	void UpdateStageStartEvent();
	void UpdateTimerEvent();
	void UpdateOrderSheetAnimation();
	void UpdateFever();

	void OrderSuccessed();
	void OrderFailed();

	bool SetOrderSheetAnime(Transform* target, const Vector3& start, const Vector3& end, float time);

	void SortOrderSheet();

	int OrderPotionRainbow();
#ifdef _EDITOR
	std::vector<std::pair<std::string, std::function<void()>>>  editorFuncButton;
#endif
	void SaveGameManagerData() const;
	void LoadGameManagerData();

	void SavePotionScoreData() const;
	void LoadPotionScoreData();

	void SaveStageData() const;
	void LoadStageData();

	void SavePotionWeightData();
	void LoadPotionWeightData();
	int potionScore[7]{};
public:

	bool IsStageStart() const { return isStageStart; }
	bool IsStageClear() const { return isStageClear; }
	bool IsTimeEnd() const { return isTimeEnd; }
	bool IsInit() const { return isinit1Tick; }

	int GetCurrentStageNum() const { return stageNum; }
	int GetLastScore() const { return lastScore; }

	/*주문 요청 실패시 id -1 반환*/
	int OrderPotion(PotionType potionType, const std::function<void()>& SuccessedCallBack, const std::function<void()>& FaildCallBack);
	/*주문서 업데이트*/
	void OrderSheetUpdate(int id, float t);
	/*주문 처리 실패시 id -1 반환*/
	int ServeOrder(PotionType potionType);
	/*걍 주문 삭제*/
	void OrderDeleteFront();

	int GetOrderCount() const { return orderQueue.size(); }

	int AddScore(PotionType potionType, int _tip);
	int AddScore(int score, int _tip = 0);
	int GetTotalScore() const;
	int GetScore() const;
	int GetTip() const;

	/*히든 업적 달성 성공*/
	void UnlockSecret();

	/*현재 획득한 별 개수*/
	int GetStarCount();

	/*남은 제한 시간*/
	float GetStageRemainingTime() const { return PlayTime - uiElapsedTimes[E_UIName::TimerUI]; }
	/*현재 시간*/
	float GetStageElapsedTime() const { return uiElapsedTimes[E_UIName::TimerUI]; }
	/*진행시간 0f~1f 사이 백분율*/
	float GetProgressPercentage() const { return uiElapsedTimes[E_UIName::TimerUI] / PlayTime; }

	void StageLoad(int stageNum);
	void StageRestart() { StageLoad(stageNum); }

	/*스테이지 클리어*/
	void StageClear();

	/*메인 메뉴로*/
	void MainMenuScene();
	/*엔딩으로 *미구현**/
	void EndingScene();
	/*게임오버로 *미구현**/
	void GameOverScene();

	/*포션 점수*/
	int GetPotionScore(PotionType potionType) const;
};