#include "BlendColorGradingControll.h"
#include "GameManager.h"
#include "PostProcessUtility.h"


void BlendColorGradingControllComponent::Awake()
{
	texturePath->resize(3);
}

void BlendColorGradingControllComponent::Start()
{

	gameManager = GameObject::FindFirst<GameManager>()->IsComponent<GameManagerComponent>();
	postProcess = &GameObject::FindFirst<PostprocessObject>()->postProcessComponent;
	elapsedTime = 0;
}

void BlendColorGradingControllComponent::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, 3);
	Binary::Write::wstring(ofs, texturePath[0]);
	Binary::Write::wstring(ofs, texturePath[1]);
	Binary::Write::wstring(ofs, texturePath[1]);
}

void BlendColorGradingControllComponent::Deserialized(std::ifstream& ifs)
{
	int size = Binary::Read::data<int>(ifs);
	texturePath[0] = Binary::Read::wstring(ifs);
	texturePath[1] = Binary::Read::wstring(ifs);
	texturePath[2] = Binary::Read::wstring(ifs);

	for (size_t i = 0; i < std::size(texturePath); i++)
	{
		texture[i] = ColorGradingTexture::Get(texturePath[i]);
	}

}

void BlendColorGradingControllComponent::InspectorImguiDraw()
{
	int loadIndex = -1;

	ImGui::Text("BlendColorGradingControll");
	ImGui::Text("elapsedTime : %f", elapsedTime);

	if (ImGui::ImageButton("Load Texture1", (ImTextureID)(ID3D11ShaderResourceView*)originTexture[0], ImVec2(200, 200)))
	{
		loadIndex = 0;
	}
	if (ImGui::ImageButton("Load Texture2", (ImTextureID)(ID3D11ShaderResourceView*)originTexture[1], ImVec2(200, 200)))
	{
		loadIndex = 1;
	}
	if (ImGui::ImageButton("Load Texture3", (ImTextureID)(ID3D11ShaderResourceView*)originTexture[2], ImVec2(200, 200)))
	{
		loadIndex = 2;
	}
	if (loadIndex != -1)
	{
		auto filePath = WinUtility::GetOpenFilePath();
		filePath = std::filesystem::relative(filePath, std::filesystem::current_path());
		texturePath[loadIndex] = filePath;
		texture[loadIndex] = ColorGradingTexture::Get(texturePath[loadIndex], &originTexture[loadIndex]);
	}

}

void BlendColorGradingControllComponent::Update()
{
	if (postProcess)
	{
		BlendColorGrading* colorGrading = postProcess->GetPostProcessData<BlendColorGrading>();
		colorGrading->SetWeight(std::fmod(elapsedTime,  0.5f) * 2);

		//시간에 따라 텍스쳐를 바꿔준다.
		if (elapsedTime < 0.5f)
		{
			colorGrading->SetTexture(texture[0], 1);
			colorGrading->SetTexture(texture[1], 2);
		}
		else
		{
			colorGrading->SetTexture(texture[1], 1);
			colorGrading->SetTexture(texture[2], 2);
		}
	}



#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay())
	{
		return;
	}
#endif // _EDITOR
	if (gameManager)
	{
		elapsedTime = gameManager->GetProgressPercentage();
	}
}
