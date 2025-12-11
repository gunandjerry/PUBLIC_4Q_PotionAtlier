#include "ImageCompressorScene.h"
#include <framework.h>

void ImageCompressorScene::PngHandler(const wchar_t* filepath)
{
	std::wstring fileName = std::filesystem::path(filepath).filename();
	
	const std::unordered_map<std::wstring, ETextureType> textureTypeMap = 
	{
		{L"Albedo", ETextureType::Albedo},
		{L"Normal", ETextureType::Normal},
		{L"Specular", ETextureType::Specular},
		{L"Emissive", ETextureType::Emissive},
		{L"Opacity", ETextureType::Opacity},
		{L"Metalness", ETextureType::Metalness},
		{L"Roughness", ETextureType::Roughness},
		{L"AmbientOcculusion", ETextureType::AmbientOcculusion},
	};
	ETextureType type = ETextureType::Null;

	for (const auto& [keyword, textureType] : textureTypeMap)
	{
		if (fileName.find(keyword) != std::wstring::npos)
		{
			type = textureType;
			break; 
		}
	}
	ImGui::ShowCompressPopup(filepath, type);
}

void ImageCompressorScene::Start()
{
	Scene::GuizmoSetting.UseImGuizmo = false;
	UseImGUI = true;
	Scene::SetDragEvent(true);
	D3D11_GameApp::SetFileHandlers(L".png", PngHandler);

	DefferdRenderer& renderer = D3D11_GameApp::GetRenderer();
	renderer.SetCameraMatrix(Matrix::Identity);
	renderer.SetOrthographicProjection(1, 100);
	renderer.Render();
}

void ImageCompressorScene::ImGUIRender()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu(ICON_FA_FOLDER_O " Image"))
		{
			if (ImGui::MenuItem(ICON_FA_FILE_IMAGE_O "  Compress Png"))
			{
				std::wstring openPath = WinUtility::GetOpenFilePath();
				if (!openPath.empty())
				{
					PngHandler(openPath.c_str());
				}			
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(ICON_FA_COG " System"))
		{
			if (ImGui::MenuItem(ICON_FA_POWER_OFF "  Exit"))
			{
				D3D11_GameApp::GameEnd();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

}
