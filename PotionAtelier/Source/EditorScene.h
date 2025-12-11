#pragma once
#include <Scene/Base/Scene.h>
#include <Component\Render\PostProcessComponent.h>
#include "PostProcessUtility.h"

class EditorScene : public Scene
{
public:
	EditorScene() = default;
	virtual ~EditorScene() override = default;
	virtual void Awake() override;

	virtual void ImGUIRender();
};