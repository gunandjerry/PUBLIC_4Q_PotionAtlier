#pragma once
#include <Core/TSingleton.h>
#include <windows.h>
#include <cmath>
#include <functional>

class QPCTime : public TSingleton<QPCTime>
{
	template <typename T>
	friend class TSingleton;
private:
	QPCTime();
	~QPCTime() = default; 

public:
	void UpdateTime();
	void UpdateDelayedInvoker();
	const int GetFrameRate() const
	{
		if (deltaTime_ms == 0) return 0;

		return static_cast<int>(std::ceil(((1000.0f / deltaTime_ms) * 1000.f) / 1000.f));
	}

	float GetDeltaTime_ms(bool isScale = true) const;
	float GetDeltaTime(bool isScale = true) const { return (GetDeltaTime_ms(isScale) / 1000.f);}

	/* DeltaTime (단위 : sec). timeScale 영향 받습니다.*/
	float GetPropertyDeltaTime() const { return deltaTime_ms * timeScale / 1000.f; }
	/* DeltaTime (단위 : sec). timeScale 영향 받습니다.*/
	__declspec(property(get = GetPropertyDeltaTime)) float DeltaTime;

	/* DeltaTime (단위 : ms). timeScale 영향 받습니다.*/
	float GetPropertyDeltaTime_ms() const { return deltaTime_ms * timeScale; }
	/* DeltaTime (단위 : ms). timeScale 영향 받습니다.*/
	__declspec(property(get = GetPropertyDeltaTime_ms)) float DeltaTime_ms;

	int FixedUpdatePerSec = 50;
	float GetFixedDelta() const { return 1.f / (float)FixedUpdatePerSec ; }

	float timeScale;

public:
	/// <summary>
	/// 전달받은 함수를 딜레이 시간 이후에 호출해줍니다. 
	/// </summary>
	/// <param name="funtion :">딜레이 이후 호출될 함수</param>
	/// <param name="delayTime :">딜레이 시간 (0.f 이면 다음 프레임에 호출.)</param>
	/// <param name="affectedTimeScale :">timeScale 적용 여부 (기본값 = true)</param>
	void DelayedInvok(const std::function<void()>& func, float delayTime, bool affectedTimeScale = true);

	//대기중인 함수들 초기화
	void ClearInvokFunc() { delayFunctionsVec.clear(); }
private:
	LARGE_INTEGER previousTime;
	LARGE_INTEGER currentTime;
	LARGE_INTEGER frequency;

	float deltaTime_ms = 0;

	//tuple<함수, 딜레이 시간, 타임스케일 여부, 진행 시간, 종료 여부>
	using DelayFunctionTuple = std::tuple<std::function<void()>, float, bool, float, bool>;
	std::vector<DelayFunctionTuple> delayFunctionsVec;
};

namespace TimeSystem
{
	extern QPCTime& Time;
}
