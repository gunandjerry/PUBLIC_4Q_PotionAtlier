#pragma once

#include <GameObject/Base/GameObject.h>
#include <Component\Render\UIRenderComponenet.h>


class UIObject : public GameObject
{
	SERIALIZED_OBJECT(UIObject)
public:
	UIObject();
	virtual ~UIObject() override = default;


	UIRenderComponenet& uiComponenet;
};


class UIMaterialObject : public GameObject
{
	SERIALIZED_OBJECT(UIMaterialObject)
public:
	UIMaterialObject();
	virtual ~UIMaterialObject() override = default;


	UIRenderComponenet2& uiComponenet;
};

