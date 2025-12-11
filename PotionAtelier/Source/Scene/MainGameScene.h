#pragma once
#include <Scene/Base/Scene.h>
#include <Manager/InputManager.h>

class MainGameScene : public Scene, public InputProcesser
{
public:
	MainGameScene();
	virtual void Awake();
	virtual ~MainGameScene();
protected:
	virtual void ImGUIRender();

private:
	virtual void OnInputProcess(InputManager::Input& input) override;
};