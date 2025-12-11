#include "LightControllObject.h"
#include <Components/LightControllComponent.h>
LightControll::LightControll()
{
	auto& light = AddComponent<LightControllComponent>();
}
