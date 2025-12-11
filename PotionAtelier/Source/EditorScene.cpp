#include "EditorScene.h"
#include <framework.h>
#include <Utility/SerializedUtility.h>
#include <Object/GameManager.h>




void EditorScene::Awake()
{
#ifndef _EDITOR
	__debugbreak(); //에디터 모드 아니면 실행 불가
	D3D11_GameApp::GameEnd();
	return;
#endif 

	Scene::GuizmoSetting.UseImGuizmo = true;

	UseImGUI = true;
	SetDragEvent(true);

	auto mainCamera = NewGameObject<CameraObject>(L"MainCamera");
	mainCamera->SetMainCamera();
	mainCamera->transform.position = Vector3(0, 0, -5);
}

void EditorScene::ImGUIRender()
{
	ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::EditHierarchyView();
	ImGui::End();

#ifdef _EDITOR
	ImGui::Begin("Shader");
	if (ImGui::Button("ShaderReload"))
	{
		std::filesystem::path shaderPath = __FILEW__;
		shaderPath = shaderPath.parent_path().parent_path() / L"Resource/EngineShader";
		ShaderUtility::CopyShader(shaderPath);
		//MeshRender::ReloadShaderAll();

		{
			ComPtr<ID3D11ComputeShader> computeShader = nullptr;
			hlslManager.ClearSharingShader(L"Resource/EngineShader/DeferredRender.hlsl");
			hlslManager.CreateSharingShader(L"Resource/EngineShader/DeferredRender.hlsl", &computeShader);
			D3D11_GameApp::GetRenderer().deferredCS.LoadShader(computeShader.Get());
		}
		{
			ComPtr<ID3D11PixelShader> pixelShader = nullptr;
			hlslManager.CreateSharingShader(L"Resource/EngineShader/CopyTexture.hlsl", &pixelShader);
			D3D11_GameApp::GetRenderer().copyTexturePS.LoadShader(pixelShader.Get());
		}
		{
			ComPtr<ID3D11VertexShader> vertexShader = nullptr;
			ComPtr<ID3D11InputLayout> inputLayout = nullptr;
			hlslManager.CreateSharingShader(L"Resource/EngineShader/FullScreenTriangle.hlsl", &vertexShader, &inputLayout);
			D3D11_GameApp::GetRenderer().fullScrennShader.LoadShader(vertexShader.Get(), inputLayout.Get());
		}
		{
			ComPtr<ID3D11PixelShader> pixelShader = nullptr;
			hlslManager.ClearSharingShader(L"Resource/EngineShader/UIPixelShader.hlsl");
			hlslManager.CreateSharingShader(L"Resource/EngineShader/UIPixelShader.hlsl", &pixelShader);
			D3D11_GameApp::GetRenderer().uiPixelShader.LoadShader(pixelShader.Get());
		}
		{
			ComPtr<ID3D11VertexShader> vertexShader = nullptr;
			ComPtr<ID3D11InputLayout> inputLayout = nullptr;
			hlslManager.ClearSharingShader(L"Resource/EngineShader/UIVertexShader.hlsl");
			hlslManager.CreateSharingShader(L"Resource/EngineShader/UIVertexShader.hlsl", &vertexShader, &inputLayout);
			D3D11_GameApp::GetRenderer().uiVertexShader.LoadShader(vertexShader.Get(), inputLayout.Get());
		}
		{
			ComPtr<ID3D11VertexShader> vertexShader = nullptr;
			ComPtr<ID3D11InputLayout> inputLayout = nullptr;
			hlslManager.ClearSharingShader(L"Resource/EngineShader/VS_Particle.hlsl");
			hlslManager.CreateSharingShader(L"Resource/EngineShader/VS_Particle.hlsl", &vertexShader, &inputLayout);
			D3D11_GameApp::GetRenderer().particleVertexShader.LoadShader(vertexShader.Get(), inputLayout.Get());
		}
		{
			ComPtr<ID3D11GeometryShader> geoMetryShader = nullptr;
			hlslManager.ClearSharingShader(L"Resource/EngineShader/GS_CreateBillboardQuad.hlsl");
			hlslManager.CreateSharingShader(L"Resource/EngineShader/GS_CreateBillboardQuad.hlsl", &geoMetryShader);
			D3D11_GameApp::GetRenderer().particleGeometryShader.LoadShader(geoMetryShader.Get());
		}
		{
			ComPtr<ID3D11ComputeShader> geoMetryShader = nullptr;
			hlslManager.ClearSharingShader(L"Resource/EngineShader/CS_Particle.hlsl");
			hlslManager.CreateSharingShader(L"Resource/EngineShader/CS_Particle.hlsl", &geoMetryShader);
			D3D11_GameApp::GetRenderer().particleComputeShader.LoadShader(geoMetryShader.Get());
		}
	}

	ImGui::End();
#endif // _DEBUG

	ImGui::Begin("FPS");
	ImGui::Text("Frame : %f", 1.0 / TimeSystem::Time.DeltaTime);
	ImGui::End();
}
