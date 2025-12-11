#pragma once
#include <GameObject/Base/GameObject.h>
#include <Component/Render/PostProcessComponent.h>


class PostprocessObject : public GameObject
{
	SERIALIZED_OBJECT(PostprocessObject)
public:
	PostprocessObject();
	virtual ~PostprocessObject() override;

	//virtual void InspectorImguiDraw();

	PostProcessComponent& postProcessComponent;
};

