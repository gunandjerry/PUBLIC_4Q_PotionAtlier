#pragma once
#include <GameObject/Mesh/UIObject.h>
#include <Component/EventListener/EventListener.h>

class ButtonObjectBase : public UIMaterialObject
{
	SERIALIZED_OBJECT(ButtonObjectBase)
public:
	ButtonObjectBase();
	virtual ~ButtonObjectBase() override = default;
	EventListener& eventListener;
};