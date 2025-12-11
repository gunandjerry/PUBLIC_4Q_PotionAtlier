#pragma once
#include "ButtonObjectBase.h"
class BoingBoingUI;
class ExitButton : public ButtonObjectBase
{
	SERIALIZED_OBJECT(ExitButton)
public:
	ExitButton();
	virtual ~ExitButton() override = default;
	BoingBoingUI& boingboing;
	virtual void Awake() override;
};