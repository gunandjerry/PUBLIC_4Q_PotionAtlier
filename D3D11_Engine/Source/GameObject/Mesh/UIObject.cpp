#include "UIObject.h"

UIObject::UIObject() :
	uiComponenet(AddComponent<UIRenderComponenet>())
{
}

UIMaterialObject::UIMaterialObject() :
	uiComponenet(AddComponent<UIRenderComponenet2>())
{
}



