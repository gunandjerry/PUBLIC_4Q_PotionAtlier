#pragma once
#include <set>
#include <Core/TSingleton.h>
#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

class UIRenderComponenet;
class UIRenderComponenet2;

enum class EventType
{
	LeftClickDown,
	LeftClickUp,
	RightClickDown,
	RightClickUp
};


class EventManager : public TSingleton<EventManager>
{
	std::vector<UIRenderComponenet*> uis1;
	std::vector<UIRenderComponenet2*> uis2;
public:
	static void AddUIRenderer(class UIRenderComponenet* ui);
	static void AddUIRenderer(class UIRenderComponenet2* ui);
	static void RemoveUIRenderer(class UIRenderComponenet* ui);
	static void RemoveUIRenderer(class UIRenderComponenet2* ui);



	static void Update();
	static void OnClickEvent(Vector2 position, EventType type);
private:
	void HandleClick(Vector2 position, EventType type);
	bool IsIntersect(Vector2 click_pos, Vector2 center, Vector2 size);
	void ActivateClickEvent(UIRenderComponenet* target, EventType type);
	void ActivateClickEvent(UIRenderComponenet2* target, EventType type);
};
