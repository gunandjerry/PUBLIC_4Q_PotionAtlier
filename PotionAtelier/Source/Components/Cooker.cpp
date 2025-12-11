#include "Cooker.h"
#include <framework.h>

Cooker::Cooker()
{
}

GameObject* Cooker::FindHoldingObject(Transform* parent)
{
	auto num = parent->GetChildCount();
	for (int i = 0; i < num; ++i)
	{
		Transform* c = parent->GetChild(i);
		if (c->gameObject.HasTag(L"HoldingObject"))
		{
			return &c->gameObject;
		}
		GameObject* obj = FindHoldingObject(c);
		if (obj != nullptr) return obj;
	}

	return nullptr;
}