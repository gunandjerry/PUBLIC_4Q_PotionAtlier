#pragma once

#include "ButtonObjectBase.h"

class BoingBoingUI;
class CutSceneSkipButton : public ButtonObjectBase
{
	SERIALIZED_OBJECT(CutSceneSkipButton)
public:
	CutSceneSkipButton();
	virtual ~CutSceneSkipButton() override = default;
	BoingBoingUI& boingboing;
	virtual void Awake() override;
};
