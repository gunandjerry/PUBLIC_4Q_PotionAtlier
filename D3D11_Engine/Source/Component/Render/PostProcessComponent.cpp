#include "PostProcessComponent.h"
#include "Manager\HLSLManager.h"
#include <framework.h>
#include <Utility/SerializedUtility.h>
#include <Utility/WinUtility.h>


PostProcessComponent::PostProcessComponent()
{
}

PostProcessComponent::~PostProcessComponent()
{
	
}

void PostProcessComponent::Awake()
{
	
}

void PostProcessComponent::InspectorImguiDraw()
{

	if (ImGui::Button("Add Post Process"))
	{
		ImGui::OpenPopup("Add Post Process");
	}

	if (ImGui::BeginPopup("Add Post Process"))
	{
		for (auto& item : PostProcessDataFactory::GetFactory())
		{
			if (ImGui::MenuItem(item.first.c_str()))
			{
				postProcessDatas.emplace_back(PostProcessDataFactory::Create(item.first));
			}
		}
		ImGui::EndPopup();
	}
	for (size_t i = 0; i < postProcessDatas.size(); i++)
	{
		ImGui::BeginChild(std::format("{}", i).c_str(),
						  ImVec2(0, 0),
						  ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoMove);
		if (ImGui::BeginPopupContextWindow())
		{
			bool isDelete = false;
			if (ImGui::Button("Delete"))
			{
				postProcessDatas.erase(postProcessDatas.begin() + i);
				ImGui::CloseCurrentPopup();
				i--;
				isDelete = true;
			}
			ImGui::EndPopup();
			if (isDelete)
			{
				ImGui::EndChild();
				continue;
			}
		}
		ImGui::Text(postProcessDatas[i]->GetTypeName().data());
		postProcessDatas[i]->InspectorImguiDraw();
		ImGui::EndChild();

	}

}

void PostProcessComponent::Serialized(std::ofstream& ofs)
{
	size_t size = postProcessDatas.size();
	Binary::Write::data(ofs, size);
	for (size_t i = 0; i < size; i++)
	{
		Binary::Write::string(ofs, postProcessDatas[i]->GetTypeName().data());
	}

	for (auto& item : postProcessDatas)
	{
		item->Serialized(ofs);
	}
}

void PostProcessComponent::Deserialized(std::ifstream& ifs)
{
	size_t size = Binary::Read::data<size_t>(ifs);
	
	for (size_t i = 0; i < size; i++)
	{
		std::string typeName = Binary::Read::string(ifs);
		postProcessDatas.emplace_back(PostProcessDataFactory::Create(typeName));
	}

	for (auto& item : postProcessDatas)
	{
		item->Deserialized(ifs);
	}
}

void PostProcessComponent::FixedUpdate()
{
}

void PostProcessComponent::Update()
{
}

void PostProcessComponent::LateUpdate()
{
}

void PostProcessComponent::Render()
{
	std::ranges::sort(postProcessDatas,
					  [](const std::shared_ptr<PostProcessData>& first, const std::shared_ptr<PostProcessData> second)
					  {
						  return first->drawSpeed < second->drawSpeed;
					  });

	for (auto& postProcessData : postProcessDatas)
	{
		D3D11_GameApp::GetRenderer().AddDrawCommand(postProcessData->postProcesCommand);
	}
}


ColorGrading::ColorGrading()
{
	drawSpeed = INT_MAX;
	postProcesCommand.computeShader.resize(1);
	postProcesCommand.computeShaderSet.resize(1);
	postProcesCommand.dispatchDatas.resize(1);

	ComPtr<ID3D11ComputeShader> shader;
	hlslManager.CreateSharingShader(L"Resource/EngineShader/ColorGrading.hlsl", &shader);
	postProcesCommand.computeShader[0].LoadShader(shader.Get());

}

ColorGrading::~ColorGrading()
{
	hlslManager.ClearSharingShader(L"Resource/EngineShader/ColorGrading.hlsl");
}

void PostProcessData::InspectorImguiDraw()
{
	ImGui::PushID(this);
	ImGui::DragInt("Draw Speed", &drawSpeed, 1);
	ImGui::PopID();
}

void ColorGrading::InspectorImguiDraw()
{
	PostProcessData::InspectorImguiDraw();
	if (ImGui::Button("Load Texture"))
	{
		SetTexture(WinUtility::GetOpenFilePath());
	}
}

void ColorGrading::Serialized(std::ofstream& ofs)
{
	Binary::Write::wstring(ofs, texturePath);
}

void ColorGrading::Deserialized(std::ifstream& ifs)
{
	texturePath = Binary::Read::wstring(ifs);
}

void ColorGrading::SetTexture(std::filesystem::path path)
{
	if (path.empty()) return;
	postProcesCommand.shaderResources.clear();

	texturePath = path;
	Texture texture = ColorGradingTexture::Get(texturePath);
	postProcesCommand.shaderResources.emplace_back(Binadble
													   {
														   .shaderType = EShaderType::Compute,
														   .bindableType = EShaderBindable::ShaderResource,
														   .slot = 10,
														   .bind = (ID3D11ShaderResourceView*)texture
													   });
}


std::shared_ptr<PostProcessData> PostProcessDataFactory::Create(std::string_view typeName)
{
	return postProcessDataFactory[typeName.data()]();
}

bool PostProcessDataFactory::Register(std::string_view typeName, std::function<std::shared_ptr<PostProcessData>()> function)
{
	postProcessDataFactory[typeName.data()] = function;
	return true;
}

Texture ColorGradingTexture::Get(std::filesystem::path texturePath, _Out_opt_ Texture* originTexture)
{
	Texture texture;
	{
		std::unique_ptr<DirectX::ScratchImage> image = std::make_unique<DirectX::ScratchImage>();
		DirectX::TexMetadata metadata;
		HRESULT hr;
		if (texturePath.extension() == ".dds")
		{
			hr = DirectX::LoadFromDDSFile(texturePath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, *image);
		}
		else
		{
			hr = DirectX::LoadFromWICFile(texturePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, *image);
		}
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Texture Load Error", L"Error", MB_OK);
			return {};
		}
		if (metadata.width % 16 != 0 || metadata.height % 16 != 0)
		{
			MessageBox(nullptr, L"Texture Size Error", L"Error", MB_OK);
			return {};
		}

		size_t sliceSize = metadata.width / 16; // 한 슬라이스의 크기
		size_t depth = 16; // 16 슬라이스

		TexMetadata metadata3D = {};
		metadata3D.width = static_cast<size_t>(sliceSize);
		metadata3D.height = static_cast<size_t>(sliceSize);
		metadata3D.depth = depth;
		metadata3D.format = metadata.format;
		metadata3D.dimension = TEX_DIMENSION_TEXTURE3D;

		std::unique_ptr<DirectX::ScratchImage> scratch3DImage = std::make_unique<DirectX::ScratchImage>();
		hr = scratch3DImage->Initialize3D(metadata3D.format, metadata3D.width, metadata3D.height, metadata3D.depth, 1);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Failed to initialize 3D texture.", L"Error", MB_OK);
			return {};
		}
		const Image* images = image->GetImages();
		const Image* images3D = scratch3DImage->GetImages();

		size_t pixelSize = BitsPerPixel(metadata.format) / 8; // 픽셀 크기 계산
		for (size_t z = 0; z < depth; ++z)
		{
			size_t offsetX = z * sliceSize;
			for (size_t y = 0; y < sliceSize; ++y)
			{
				const uint8_t* srcRow = images->pixels + y * images->rowPitch + offsetX * pixelSize;
				uint8_t* dstRow = images3D->pixels + y * images3D->rowPitch + z * images3D->slicePitch;

				memcpy(dstRow, srcRow, sliceSize * pixelSize);
			}
		}
		
		texture.CreateTexture(std::move(scratch3DImage), ETextureUsage::SRV);
		if (originTexture)
		{
			originTexture->CreateTexture(std::move(image), ETextureUsage::SRV);
		}
	}
	return texture;
}

ToneMapping::ToneMapping()
{
	drawSpeed = 0;
	postProcesCommand.computeShader.resize(1);
	postProcesCommand.computeShaderSet.resize(1);
	postProcesCommand.dispatchDatas.resize(1);
	ComPtr<ID3D11ComputeShader> shader;
	hlslManager.CreateSharingShader(L"Resource/EngineShader/ToneMapping.hlsl", &shader);
	postProcesCommand.computeShader[0].LoadShader(shader.Get());


	cnstantBuffer.Set(value);
	postProcesCommand.shaderResources.emplace_back(Binadble
												   {
													   .shaderType = EShaderType::Compute,
													   .bindableType = EShaderBindable::ConstantBuffer,
													   .slot = 0,
													   .bind = (ID3D11Buffer*)cnstantBuffer
												   });

}

ToneMapping::~ToneMapping()
{
	hlslManager.ClearSharingShader(L"Resource/EngineShader/ToneMapping.hlsl");
}

void ToneMapping::InspectorImguiDraw()
{
	PostProcessData::InspectorImguiDraw();
	bool isChanged = false;
	isChanged |= ImGui::Combo("Tone Mapping Type", &value.toneMappingType, "None\0Reinhard\0Filmic\0Uncharted2\0");
	isChanged |= ImGui::DragFloat("Exposure", &value.exposure, 0.1f);
	if(isChanged)
	{
		cnstantBuffer.Set(value);
	}
}

void ToneMapping::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, value.toneMappingType);
	Binary::Write::data(ofs, value.exposure);
}

void ToneMapping::Deserialized(std::ifstream& ifs)
{
	value.toneMappingType = Binary::Read::data<int>(ifs);
	value.exposure = Binary::Read::data<float>(ifs);
	cnstantBuffer.Set(value);
}
