#include "PostProcessObject.h"

PostprocessObject::PostprocessObject() :
	postProcessComponent(AddComponent<PostProcessComponent>())
{

}

PostprocessObject::~PostprocessObject()
{
}
