#include "MainGameScene.h"
#include <Manager/SceneManager.h>s
#include "Object/GameManager.h"

MainGameScene::MainGameScene()
{

}

void MainGameScene::Awake()
{
	sceneManager.LoadScene(L"EngineResource/MainMenu/MainMenu.Scene");
}

MainGameScene::~MainGameScene()
{
}

void MainGameScene::ImGUIRender()
{
	ImGui::Begin("GameManager");
	{
		if (GameManagerComponent* gameManager = GameManager::IsGM())
		{
			gameManager->InspectorImguiDraw();
		}		
	}
	ImGui::End();                      
}

void MainGameScene::OnInputProcess(InputManager::Input& input)
{
	if (input.IsKeyDown(KeyboardKeys::F10))
	{
		UseImGUI = !UseImGUI;
	}
}
