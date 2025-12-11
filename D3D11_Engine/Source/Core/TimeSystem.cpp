#include "TimeSystem.h"

namespace TimeSystem
{
	QPCTime& Time = QPCTime::GetInstance();
}

QPCTime::QPCTime() :
	timeScale(1.0f)
{
	previousTime = { 0 };
	currentTime = { 0 };
	frequency = { 0 };
	deltaTime_ms = 0.f;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&previousTime);
}

void QPCTime::UpdateTime()
{
	previousTime = currentTime;
	QueryPerformanceCounter(&currentTime);

	deltaTime_ms = float(currentTime.QuadPart - previousTime.QuadPart) / float(frequency.QuadPart / 1000.0); //ms
}

float QPCTime::GetDeltaTime_ms(bool isScale) const
{
	if (isScale)
	{
		return deltaTime_ms * timeScale;
	}
	else
	{
		return deltaTime_ms;
	}
	
}

void QPCTime::UpdateDelayedInvoker()
{
	if (!delayFunctionsVec.empty())
	{
		bool hasEnd = false;
		for (auto& [func, delayTime, useTimeScale, elapsedTime, isEnd] : delayFunctionsVec)
		{
			useTimeScale ? elapsedTime += GetDeltaTime(true) : elapsedTime += GetDeltaTime(false);
			if (delayTime <= elapsedTime)
			{
				func();
				isEnd = true;
				hasEnd = true;
			}
		}
		if (hasEnd)
		{
			std::erase_if(delayFunctionsVec, [](DelayFunctionTuple& parameters)
				{
					return std::get<4>(parameters);
				});
		}
	}
}

void QPCTime::DelayedInvok(const std::function<void()>& func, float delayTime, bool affectedTimeScale)
{
	if (delayTime <= 0.f)
	{
		delayTime = 0.f;
	}
	delayFunctionsVec.emplace_back(func, delayTime, affectedTimeScale, 0.f, false);
}


