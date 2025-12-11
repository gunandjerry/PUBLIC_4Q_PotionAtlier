#include "EventManager.h"
#include "Component/Render/UIRenderComponenet.h"
#include "Component/EventListener/EventListener.h"

void EventManager::AddUIRenderer(UIRenderComponenet* ui)
{
	auto& uiVec = GetInstance().uis1;
	uiVec.push_back(ui);
}
void EventManager::AddUIRenderer(UIRenderComponenet2* ui)
{
	auto& uiVec = GetInstance().uis2;
	uiVec.push_back(ui);
}

void EventManager::RemoveUIRenderer(UIRenderComponenet* ui)
{
	std::erase(GetInstance().uis1, ui);
}
void EventManager::RemoveUIRenderer(UIRenderComponenet2* ui)
{
	std::erase(GetInstance().uis2, ui);
}

void EventManager::Update()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay())
		return;
#endif // _EDITOR
	if (DXTKinputSystem.IsKeyDown(MouseKeys::leftButton))
	{
		auto ms = DXTKinputSystem.GetMouseState();
		OnClickEvent({ (float)ms.x, (float)ms.y }, EventType::LeftClickDown);
	}
	else if (DXTKinputSystem.IsKeyUp(MouseKeys::leftButton))
	{
		auto ms = DXTKinputSystem.GetMouseState();
		OnClickEvent({ (float)ms.x, (float)ms.y }, EventType::LeftClickUp);
	}
	if (DXTKinputSystem.IsKeyDown(MouseKeys::rightButton))
	{
		auto ms = DXTKinputSystem.GetMouseState();
		OnClickEvent({ (float)ms.x, (float)ms.y }, EventType::RightClickDown);
	}
	else if (DXTKinputSystem.IsKeyUp(MouseKeys::rightButton))
	{
		auto ms = DXTKinputSystem.GetMouseState();
		OnClickEvent({ (float)ms.x, (float)ms.y }, EventType::RightClickUp);
	}
}

void EventManager::OnClickEvent(Vector2 position, EventType type)
{
	GetInstance().HandleClick(position, type);
}

void EventManager::HandleClick(Vector2 position, EventType type)
{
	std::vector<UIRenderComponenet*> valid_ui1;
	std::vector<UIRenderComponenet2*> valid_ui2;
	std::vector<GameObject*> OutVector;
	for (auto* ui : uis1)
	{
		/*bool isActiveDisable = false;
		ui->gameObject.GetHierarchyToParent(OutVector);
		for (auto& item : OutVector)
		{
			if (item->Active == false)
			{
				isActiveDisable = true;
				break;
			}
		}
		if (isActiveDisable)
		{
			continue;
		}*/
		if (ui->Enable == true && ui->gameObject.Active == true && ui->texture.GetSRV() != nullptr)
		{
			valid_ui1.push_back(ui);
		}
	}
	for (auto* ui : uis2)
	{
		/*bool isActiveDisable = false;
		ui->gameObject.GetHierarchyToParent(OutVector);
		for (auto& item : OutVector)
		{
			if (item->Active == false)
			{
				isActiveDisable = true;
				break;
			}
		}
		if (isActiveDisable)
		{
			continue;
		}*/
		if (ui->Enable == true && ui->gameObject.Active == true && ui->materialAsset.GetTexturesV2().size() > 0 && ui->this_is_mask == false)
		{
			valid_ui2.push_back(ui);
		}
	}

	std::sort(valid_ui1.begin(), valid_ui1.end(), [](UIRenderComponenet* a, UIRenderComponenet* b)
	{
		return a->drawSpeed > b->drawSpeed;
	});
	std::sort(valid_ui2.begin(), valid_ui2.end(), [](UIRenderComponenet2* a, UIRenderComponenet2* b)
	{
		return a->drawSpeed > b->drawSpeed;
	});

	auto iter1 = valid_ui1.begin();
	auto iter2 = valid_ui2.begin();
	while (iter1 != valid_ui1.end() || iter2 != valid_ui2.end())
	{
		int select_type = -1;
		float min_speed = -FLT_MAX;

		if (iter1 != valid_ui1.end() && (*iter1)->drawSpeed > min_speed && IsIntersect(position, (*iter1)->GetCenterPosition(), (*iter1)->GetSize()))
		{
			select_type = 0;
			min_speed = (*iter1)->drawSpeed;
		}
		if (iter2 != valid_ui2.end() && (*iter2)->drawSpeed > min_speed && IsIntersect(position, (*iter2)->GetCenterPosition(), (*iter2)->GetSize()))
		{
			select_type = 1;
			min_speed = (*iter2)->drawSpeed;
		}

		if (select_type == 0)
		{
			ActivateClickEvent(*iter1, type);
			break;
		}
		else if (select_type == 1)
		{
			ActivateClickEvent(*iter2, type);
			break;
		}

		if (iter1 != valid_ui1.end()) ++iter1;
		if (iter2 != valid_ui2.end()) ++iter2;
	}	
}

bool EventManager::IsIntersect(Vector2 click_pos, Vector2 center, Vector2 size)
{
	Vector2 mi{ center.x - (size.x * 0.5f), center.y - (size.y * 0.5f) };
	Vector2 mx{ center.x + (size.x * 0.5f), center.y + (size.y * 0.5f) };

	if (click_pos.x >= mi.x && click_pos.x <= mx.x && click_pos.y >= mi.y && click_pos.y <= mx.y)
	{
		return true;
	}
	return false;
}

void EventManager::ActivateClickEvent(UIRenderComponenet* target, EventType type)
{
	if (EventListener* listener = target->gameObject.IsComponent<EventListener>(); listener != nullptr)
	{
		switch (type)
		{
		case EventType::LeftClickDown:
			listener->InvokeOnClickDown();
			break;
		case EventType::LeftClickUp:
			listener->InvokeOnClickUp();
			break;
		case EventType::RightClickDown:
			listener->InvokeOnRightClickDown();
			break;
		case EventType::RightClickUp:
			listener->InvokeOnRightClickUp();
			break;
		}
	}
}

void EventManager::ActivateClickEvent(UIRenderComponenet2* target, EventType type)
{
	if (EventListener* listener = target->gameObject.IsComponent<EventListener>(); listener != nullptr)
	{
		switch (type)
		{
		case EventType::LeftClickDown:
			listener->InvokeOnClickDown();
			break;
		case EventType::LeftClickUp:
			listener->InvokeOnClickUp();
			break;
		case EventType::RightClickDown:
			listener->InvokeOnRightClickDown();
			break;
		case EventType::RightClickUp:
			listener->InvokeOnRightClickUp();
			break;
		}
	}
}
