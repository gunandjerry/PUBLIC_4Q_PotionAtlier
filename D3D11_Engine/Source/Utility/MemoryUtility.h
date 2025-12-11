#pragma once
#include <intrin.h>
#include <memory>

namespace Utility	
{
	template<typename T>
	unsigned long SafeRelease(T& p)
	{
		if (p)
		{
			unsigned long refCount = p->Release();
			p = nullptr;
			return refCount;
		}
		return -1;
	}
	template<typename T>
	void SafeReleaseArray(T& pArr)
	{
		for (auto& p : pArr)
		{
			if (p)
			{
				p->Release();
				p = nullptr;
			}
		}
		return;
	}

	template<typename T>
	void SafeDelete(T& p)
	{
		if (p)
		{
			delete p;
			p = nullptr;
		}
	}
}