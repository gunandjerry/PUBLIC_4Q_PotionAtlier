#include "MeshRender.h"
#include <Manager/ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Manager/TextureManager.h>
#include <Utility/MemoryUtility.h>
#include <ranges>
#include <format>
#include <Utility/ImguiHelper.h>
#include <Manager/SceneManager.h>
#include <NodeEditor/NodeEditor.h>
#include <Utility/SQLiteLogger.h>
#include <Utility/SerializedUtility.h>

using namespace Utility;

void MeshRender::ReloadShaderAll()
{
	for (auto& item : instanceList)
	{		
		item->ReloadShader();
	}	
}

void MeshRender::ExportMaterialAll()
{
#ifdef _EDITOR
	static std::string lodingInfoString;
	static std::atomic_bool endExport;
	static std::mutex lodingMutex;
	static float  loading_progress; 
	loading_progress = 0.f;
	endExport = false;
	sceneManager.SetLodingImguiFunc([]()
		{   
			lodingMutex.lock();
			std::string text = lodingInfoString;
			lodingMutex.unlock();
			ImGui::OpenPopup("Export Materials");
			ImGuiIO& io = ImGui::GetIO();
			if (ImGui::BeginPopupModal("Export Materials", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				// 진행 상태 표시
				ImGui::Text(text.c_str());
				ImGui::ProgressBar(loading_progress); // Progress Bar
				
				// 작업이 끝났을 때 팝업 닫기
				if (endExport)
				{
					ImGui::CloseCurrentPopup();		
					sceneManager.EndLodingImguiFunc();
				}
				ImGui::SetWindowPos(ImVec2
					(io.DisplaySize.x * 0.5f - ImGui::GetWindowWidth() * 0.5f, 
					 io.DisplaySize.y * 0.5f - ImGui::GetWindowHeight() * 0.5f));
				ImGui::EndPopup();
			}
		});

	auto threadFunc = []()
		{
			Utility::CheckHRESULT(CoInitializeEx(nullptr, COINIT_MULTITHREADED)); //작업 스레드의 Com 객체 사용 활성화
			int i = 1;
			int count = instanceList.size();
			for (auto& item : instanceList)
			{
				lodingMutex.lock();
				lodingInfoString = "Export : ";
				lodingInfoString += utfConvert::wstring_to_utf8(item->materialAsset.GetAssetPath());
				loading_progress = (float)i / (float)count;
				lodingMutex.unlock();
				item->ExportMaterialNode();
				item->ReloadShader();
				++i;
			}
			CoUninitialize();
			endExport = true;
		};
	std::thread exportThread(threadFunc);
	exportThread.detach();
#endif //_EDITOR
}

MeshRender::MeshRender()
{
	instanceList.push_back(this);
	materialAsset.OpenAsset(L"Resource/MeshRenderTemp/MeshRender.MaterialAsset");
}

MeshRender::~MeshRender()
{
	std::erase(instanceList, this);
	meshDrawCommand.~MeshDrawCommand();
}

void MeshRender::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	Write::wstring(ofs, materialAsset.GetAssetPath());
	materialAsset.SaveAsset();
	Write::wstring(ofs, defaultMaterialPath);
	Write::data(ofs, Enable);
	Write::Color(ofs, baseColor);
	Write::string(ofs, GetMeshID());
	Write::wstring(ofs, GetVSpath());
	Write::wstring(ofs, GetPSpath());

	std::filesystem::path savePath = GetMeshResourcePath();
	if (!savePath.empty())
	{
		SaveMeshResource(savePath);
	}
}

void MeshRender::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	std::wstring AssetPath = Read::wstring(ifs);
	materialAsset.OpenAsset(AssetPath.c_str());
	defaultMaterialPath = Read::wstring(ifs);
	Enable = Read::data<bool>(ifs);
	baseColor = Read::Color(ifs);
	SetMeshID(Read::string(ifs));    //meshID
	SetVS(Read::wstring(ifs).c_str());
	SetPS(Read::wstring(ifs).c_str());
}

void MeshRender::InspectorImguiDraw()
{
	if (Scene* scene = sceneManager.GetActiveScene())
	{
		ImGui::PushID(GetComponentIndex());
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.3f, 0.4f, 1.0f)); // 배경색
		ImGui::BeginChild("MeshRenderChild", ImVec2(0, 0), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
		if (ImGui::TreeNode("MeshRender Component"))
		{
			if (ImGui::Button("Edit Material"))
			{
				std::wstring editorPath = MakeDefaultMaterialNodePath(defaultMaterialPath.c_str());
				ShaderNodeEditor* editor = scene->MakeShaderNodeEditor(editorPath.c_str());
				editor->EndPopupEvent = [scene, editorPath]()
					{
						scene->EraseShaderNodeEditor(editorPath.c_str());
					};
			}
			if (ImGui::Button("Load Material Asset"))
			{
				materialAsset.OpenAssetWithDialog();
			}	
			if(ImGui::Button(utfConvert::wstring_to_utf8(materialAsset.GetAssetPath()).c_str()))
			{
				materialAsset.ClearCache();
				materialAsset.OpenAsset(materialAsset.GetAssetPath().c_str());
			}

			for (auto& item : materialAsset.customData.GetFieldData())
			{
				if (item.second.size == 4)
				{
					ImGui::DragFloat(item.first.c_str(), materialAsset.customData.GetField<float>(item.first));
				}
				else if (item.second.size == 8)
				{
					ImGui::DragVector2(item.first.c_str(), materialAsset.customData.GetField<Vector2>(item.first));
				}
				else if (item.second.size == 12)
				{
					ImGui::DragVector3(item.first.c_str(), materialAsset.customData.GetField<Vector3>(item.first));
				}
				else if (item.second.size == 16)
				{
					ImGui::DragVector4(item.first.c_str(), materialAsset.customData.GetField<Vector4>(item.first));
				}

			}

			ImGui::TreePop();
		}
		ImGui::EndChild(); // Child 끝내기
		ImGui::PopStyleColor(); // 스타일 복구
		ImGui::PopID();
	}
}

std::pair<bool, bool> MeshRender::ExistsDefaultMaterialNodeAndAsset(const wchar_t* materialPath) const
{
	std::filesystem::path nodePath = MakeDefaultMaterialNodePath(materialPath);
	bool isNode = std::filesystem::exists(nodePath);
	std::filesystem::path assetPath = MakeDefaultMaterialAssetPath(materialPath);
	bool isAsset = std::filesystem::exists(assetPath);
	return std::make_pair(isNode, isAsset);
}

void MeshRender::CreateDefaultMaterial(const wchar_t* materialPath)
{
#ifdef _EDITOR
	if (MeshID.empty())
	{
		Debug_printf("MeshRender::CreateDefaultMaterial Error : MeshID is empty. Please call SetMeshID first.\n");
		SQLiteLogger::EditorLog("Error", "MeshRender::CreateDefaultMaterial Error : MeshID is empty. Please call SetMeshID first.");
		return;
	}
	
	defaultMaterialPath = materialPath;
	std::filesystem::path nodePath = MakeDefaultMaterialNodePath(defaultMaterialPath.c_str());
	std::unique_ptr<ShaderNodeEditor> editor = std::make_unique<ShaderNodeEditor>(nodePath);
	if (!std::filesystem::exists(nodePath))
	{
		std::filesystem::create_directories(nodePath.parent_path());
		DefaultMaterialEvent(editor);
		editor->Save();
	}

	std::filesystem::path assetPath = MakeDefaultMaterialAssetPath(defaultMaterialPath.c_str());
	if (!std::filesystem::exists(assetPath))
	{
		std::filesystem::create_directories(assetPath.parent_path());
		editor->Export(assetPath.c_str());
	}
	materialAsset.OpenAsset(assetPath.c_str());
#endif // _EDITOR
}

std::unique_ptr<ShaderNodeEditor> MeshRender::GetDefaultNodeEditor() const
{
#ifdef _EDITOR
	if (!defaultMaterialPath.empty())
	{
		std::filesystem::path nodePath = MakeDefaultMaterialNodePath(defaultMaterialPath.c_str());
		return std::make_unique<ShaderNodeEditor>(nodePath);
	}
	else
	{
		Debug_printf("MeshRender::GetDefaultNodeEditor Error : defaultMaterialNodePath is empty. Please call CreateDefaultMaterial first.\n");
		SQLiteLogger::EditorLog("Error", "MeshRender::GetDefaultNodeEditor Error : defaultMaterialNodePath is empty. Please call CreateDefaultMaterial first.");
	}
#endif // _EDITOR
	return nullptr;
}

void MeshRender::ExportMaterialNode()
{
#ifdef _EDITOR
	std::unique_ptr<ShaderNodeEditor> defaultNodeEditor = GetDefaultNodeEditor();
	const std::wstring& assetPath = materialAsset.GetAssetPath();
	if (!assetPath.empty())
	{
		defaultNodeEditor->Export(materialAsset.GetAssetPath(), true);
		ReloadShader();
	}
	else
	{
		Debug_printf("MeshRender::ExportMaterialNode Warning : AssetPath is empty. Please call materialAsset.OpenAsset first.\n");
		SQLiteLogger::EditorLog("Warning", "MeshRender::ExportMaterialNode Warning : AssetPath is empty. Please call materialAsset.OpenAsset first.");
	}
#endif // _EDITOR
}

void MeshRender::ReloadShader()
{
	hlslManager.ClearSharingShader(currVSpath.c_str());
	hlslManager.ClearSharingShader(currPSpath.c_str());

	meshDrawCommand.meshData.vertexShader.LoadShader(nullptr, nullptr);
	meshDrawCommand.materialData.pixelShader.LoadShader(nullptr);

	SetVS(currVSpath.c_str());
	SetPS(currPSpath.c_str());

	materialAsset.OpenAsset(materialAsset.GetAssetPath().c_str());
}

void MeshRender::Render()
{
	//초기화
	meshDrawCommand.meshData.shaderResources.clear();
	meshDrawCommand.materialData.shaderResources.clear();

	//TransformBufferData 업데이트(더티 체크 필요할듯?)
	transformBuffer.Set(
		TransformBufferData
		{
			.World = XMMatrixTranspose(transform.GetWM()),
			.WorldInverseTranspose = transform.GetIWM()
		},
		true
	);

	//등록
	meshDrawCommand.meshData.shaderResources.push_back(
		Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)transformBuffer
		}
	);
	meshDrawCommand.meshData.shaderResources.push_back(
		Binadble
		{
			.shaderType = EShaderType::Pixel,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)transformBuffer
		}
	);

	//텍스쳐 등록
	size_t textureCount = materialAsset.GetTexturesV2().size();
	const auto& textures = materialAsset.GetTexturesV2();
	const auto& textureSlot = materialAsset.GetTexturesSlot();
	for (size_t i = 0; i < textureCount; i++)
	{
		Binadble bind{};
		bind.bindableType = EShaderBindable::ShaderResource;
		bind.shaderType = EShaderType::Pixel;
		bind.slot = textureSlot[i];
		bind.bind = (ID3D11ShaderResourceView*)textures[i];
		meshDrawCommand.materialData.shaderResources.push_back(bind);
	}

	//샘플러 등록
	size_t samplersCount = materialAsset.GetSamplers().size();
	const auto& samplers = materialAsset.GetSamplers();
	const auto& samplerSlot = materialAsset.GetSamplerSlot();
	for (size_t i = 0; i < samplersCount; i++)
	{
		Binadble bind{};
		bind.bindableType = EShaderBindable::Sampler;
		bind.shaderType = EShaderType::Pixel;
		bind.slot = samplerSlot[i];
		bind.bind = (ID3D11SamplerState*)samplers[i];
		meshDrawCommand.materialData.shaderResources.push_back(bind);
	}

	meshDrawCommand.materialData.pixelShader = materialAsset.GetPS();

	if (materialAsset.GetCustomBuffer())
	{
		materialAsset.GetCustomBuffer().Update(materialAsset.customData.Data());
		meshDrawCommand.materialData.shaderResources.push_back(
			Binadble
			{
				.shaderType = EShaderType::Pixel,
				.bindableType = EShaderBindable::ConstantBuffer,
				.slot = 5,
				.bind = (ID3D11Buffer*)materialAsset.GetCustomBuffer()
			}
		);
	}

	//바운딩 박스
	meshDrawCommand.meshData.boundingBox = gameObject.GetOBBToWorld();

	//업데이트 호출
	UpdateMeshDrawCommand();

	//드로우 요청
	D3D11_GameApp::GetRenderer().AddDrawCommand(meshDrawCommand);
}

void MeshRender::SetVS(const wchar_t* path)
{
	currVSpath = path;
	{
		ComPtr<ID3D11VertexShader> vs;
		ComPtr<ID3D11InputLayout> il;
		hlslManager.CreateSharingShader(currVSpath.c_str(), &vs, &il);
		meshDrawCommand.meshData.vertexShader.LoadShader(vs.Get(), il.Get());
	}
}

void MeshRender::SetPS(const wchar_t* path)
{
	currPSpath = path;
	{
		ComPtr<ID3D11PixelShader> ps;
		hlslManager.CreateSharingShader(currPSpath.c_str(), &ps);
		meshDrawCommand.materialData.pixelShader.LoadShader(ps.Get());
	}
}

void MeshRender::SetMeshID(const std::string& meshID)
{
	if (meshID.empty())
	{
		Debug_printf("MeshRender::SetMeshID Error : meshID is Invalid argument provided.\n");
		SQLiteLogger::GameLog("Error", "MeshRender::SetMeshID Error : meshID is Invalid argument provided.");
		return;
	}
	else
	{
		MeshID = meshID;
	}

	if (!defaultMaterialPath.empty())
	{	
		std::filesystem::path meshResourcePath = GetMeshResourcePath();
		LoadMeshResource(meshResourcePath);
	}
}

std::filesystem::path MeshRender::GetMeshResourcePath() const
{
	if (defaultMaterialPath.empty() || MeshID.empty())
	{
		return std::filesystem::path();
	}

	std::filesystem::path meshPath = L"BinaryData/MeshAsset";
	meshPath /= defaultMaterialPath;
	meshPath /= MeshID;
	meshPath += L".MeshAsset";
	return meshPath;
}

std::filesystem::path MeshRender::MakeDefaultMaterialNodePath(const wchar_t* materialPath) const
{
	if (MeshID.empty())
	{
		Debug_printf("MeshRender::MakeDefaultMaterialNodePath Error : MeshID is empty. Please call SetMeshID first.\n");
		SQLiteLogger::EditorLog("Error", "MeshRender::MakeDefaultMaterialNodePath Error : MeshID is empty. Please call SetMeshID first.");
		return std::filesystem::path();
	}

	std::filesystem::path nodePath = L"Resource/Materials";
	nodePath /= materialPath;
	nodePath /= MeshID;
	nodePath += L".Proj";
	return nodePath;
}

std::filesystem::path MeshRender::MakeDefaultMaterialAssetPath(const wchar_t* materialPath) const
{
	if (MeshID.empty())
	{
		Debug_printf("MeshRender::MakeDefaultMaterialAssetPath Error : MeshID is empty. Please call SetMeshID first.\n");
		SQLiteLogger::GameLog("Error", "MeshRender::MakeDefaultMaterialAssetPath Error : MeshID is empty. Please call SetMeshID first.");
		return std::filesystem::path();
	}

	std::filesystem::path assetPath = L"BinaryData/Materials";
	assetPath /= materialPath;
	assetPath /= MeshID;
	assetPath += L".MaterialAsset";
	return assetPath;
}

