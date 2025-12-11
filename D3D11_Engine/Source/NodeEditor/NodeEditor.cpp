#include "NodeEditor.h"
#include "json.hpp"
#include "ShaderNodes.h"
#include <ranges>
#include <chrono>
#include <set>
#include "Utility/WinUtility.h"
#include "Asset\MaterialAsset.h"
#include <Utility/utfConvert.h>
#include <Component/Render/MeshRender.h>

class NodeEditorManager
{
public:
	NodeEditorManager()
	{
		auto textureHandler = [this](const wchar_t* filePath)
			{
				std::wstring path = filePath;
				for (auto& item : editors)
				{
					item->tickEvents.emplace_back(
						[path, item]()
						{
							if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
							{
								auto newNode = item->myGrid->Create<TextureNode>("TextureNode");
								newNode->Set(path);
							}
						});
				}
			};

		D3D11_GameApp::SetFileHandlers(L".png", textureHandler);
		D3D11_GameApp::SetFileHandlers(L".dds", textureHandler);
	}

	~NodeEditorManager()
	{
		D3D11_GameApp::RemoveFileHandlers(L".png");
		D3D11_GameApp::RemoveFileHandlers(L".dds");
	}

	static std::shared_ptr<NodeEditorManager> GetInstance()
	{
		static std::weak_ptr<NodeEditorManager> instance;
		std::shared_ptr<NodeEditorManager> m_instance;
		m_instance = instance.lock();
		if (!m_instance)
		{
			m_instance = std::make_shared<NodeEditorManager>();
			instance = m_instance;
		}

		return m_instance;
	}

	void AddNodeEditor(NodeEditor* editor)
	{
		editors.push_back(editor);
	}

	void RemoveNodeEditor(NodeEditor* editor)
	{
		editors.remove(editor);
	}

private:
	std::list<NodeEditor*> editors;
};



std::shared_ptr<ShaderNode> NodeFlow::Create(std::string_view typeName)
{
	return nodeFactory.Create(typeName);
}

NodeEditor::NodeEditor() : path{}
{
	manager = NodeEditorManager::GetInstance();
	manager->AddNodeEditor(this);
}

NodeEditor::~NodeEditor()
{
	if (willSave && myGrid)
	{
		Save();
		// 모든 노드 저장
	}

	myGrid.reset();
	NodeEditorManager::GetInstance()->RemoveNodeEditor(this);
}

void NodeEditor::Update()
{

	ImGui::Begin(fileName.c_str(), &isPopUp, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);
	for (auto& item : tickEvents)
	{
		item();

	}
	tickEvents.clear();

	if (!isPopUp)
	{
		ImGui::OpenPopup("Save?");
	}
	if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_None))
	{
		if (ImGui::Button("Save"))
		{
			willSave = true;
			ImGui::CloseCurrentPopup();


			if (EndPopupEvent)
			{
				EndPopupEvent();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Don't Save"))
		{
			willSave = false;
			ImGui::CloseCurrentPopup();


			if (EndPopupEvent)
			{
				EndPopupEvent();
			}
		}

		ImGui::EndPopup();
	}

	UpdateImp();
	ImGui::BeginGroup();
	ImGui::PushItemWidth(200);
	{
		ImGui::Text("Shading Model");
		ImGui::PushID("shadingModel");
		ImGui::Combo("", (int*)&myGrid->shadingModel, EShadingModel::name, IM_ARRAYSIZE(EShadingModel::name));
		ImGui::PopID();

		ImGui::Text("BlendMode");
		ImGui::PushID("blendMode");
		ImGui::Combo("", (int*)&myGrid->blendMode, EBlendMode::name, IM_ARRAYSIZE(EBlendMode::name));
		ImGui::PopID();

		ImGui::Checkbox("IsParticle", &isParticle);
		
		for (size_t i = 0; i < myGrid->customData.size(); i++)
		{

			ImGui::BeginChild(std::format("[{}] object", i).c_str(), ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoMove);

			//드래그앤드랍 시작
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("ShaderCustomData", &i, sizeof(size_t));
				ImGui::Text(myGrid->customData[i].name.c_str());
				ImGui::EndDragDropSource();
			}
			ImGui::PushItemWidth(200);
			ImGui::PushID("ShaderCustomDataName");
			ImGui::InputText("", myGrid->customData[i].name.data(), myGrid->customData[i].name.size() + 1, ImGuiInputTextFlags_CallbackResize,
							 [](ImGuiInputTextCallbackData* data) -> int
							 {
								 std::string* str = (std::string*)data->UserData;
								 str->resize(data->BufTextLen);
								 memcpy(&(*str)[0], data->Buf, data->BufTextLen);
								 return 0;
							 }, & myGrid->customData[i].name);
			ImGui::PopID();

			ImGui::PushID("ShaderCustomDataCombo");
			ImGui::Combo("", (int*)&myGrid->customData[i].type, ECustomDataTyoe::name, IM_ARRAYSIZE(ECustomDataTyoe::name));
			ImGui::PopID();

			if (ImGui::Button((char*)u8"삭제"))
			{
				myGrid->customData.erase(myGrid->customData.begin() + i);
				--i;
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();
		}

		if (ImGui::Button((char*)u8"추가"))
		{
			myGrid->customData.emplace_back();
			myGrid->customData.back().name = std::format("CustomData{}", myGrid->customData.size());
			myGrid->customData.back().guid = std::hash<std::string>()(myGrid->customData.back().name + std::to_string(myGrid->customData.size()));
		}

	}
	ImGui::PopItemWidth();
	ImGui::EndGroup();

	ImGui::SameLine();
	ImGui::BeginChild("에에엑", ImGui::GetContentRegionAvail() - ImVec2(200.0f, 0.0f), ImGuiChildFlags_Borders);

	myGrid->update();

	ImVec2 mousePos = ImGui::GetMousePos();  // 마우스 위치
	// 마우스 클릭 및 드래그 시작 확인
	if (ImGui::IsMouseClicked(0))
	{
		auto& grid = myGrid->getGrid();
		//마우스가 그리드안인지 확인
		if (mousePos.x > grid.origin().x && mousePos.y > grid.origin().y &&
			mousePos.x < grid.origin().x + grid.size().x && mousePos.y < grid.origin().y + grid.size().y)
		{
			dragStartPos = (ImGui::GetMousePos());
			isDragging = true;
		}
	}
	if (myGrid->on_selected_node())
	{
		isDragging = false;
	}

	// 드래그 중일 때
	if (isDragging)
	{
		dragEndPos = (ImGui::GetMousePos());

		// 드래그 영역을 그리기
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		float minX = (std::min)(dragStartPos.x, dragEndPos.x);
		float maxX = (std::max)(dragStartPos.x, dragEndPos.x);
		float minY = (std::min)(dragStartPos.y, dragEndPos.y);
		float maxY = (std::max)(dragStartPos.y, dragEndPos.y);

		//화면 영역만

		draw_list->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), IM_COL32(255, 255, 255, 255));


		// 마우스가 놓였을 때 선택 처리
		if (ImGui::IsMouseReleased(0))
		{
			isDragging = false;

			dragStartPos = myGrid->screen2grid(dragStartPos);
			dragEndPos = myGrid->screen2grid(dragEndPos);
			minX = (std::min)(dragStartPos.x, dragEndPos.x);
			maxX = (std::max)(dragStartPos.x, dragEndPos.x);
			minY = (std::min)(dragStartPos.y, dragEndPos.y);
			maxY = (std::max)(dragStartPos.y, dragEndPos.y);

			for (ImFlow::BaseNode* node : myGrid->getNodes() | std::views::transform([](auto& item) { return item.second.get(); }))
			{
				ImVec2 nodePos = node->getPos();
				auto nodeSize = node->getSize();

				// 노드와 드래그 박스가 겹치는지 확인
				if (nodePos.x < maxX && nodePos.x + nodeSize.x > minX &&
					nodePos.y < maxY && nodePos.y + nodeSize.y > minY)
				{
					node->selected(true);
				}
			}

			for (const auto& link : myGrid->getLinks() | std::views::transform([](const auto& item) { return item.lock().get(); }))
			{
				link->DragSelect({ minX, minY }, { maxX, maxY });
			}
		}

	}


	ImGui::EndChild();


	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ShaderCustomData"))
		{
			size_t index = *(size_t*)payload->Data;
			auto newNode = myGrid->Create<CustomValueNode>("CustomValueNode");
			newNode->Set(myGrid->customData[index].guid);
		}
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NodeType"))
		{
			std::string typeName = (const char*)payload->Data;
			myGrid->Create(typeName.c_str());
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();

	ImGui::BeginChild("Palette", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoMove);

	ImGui::InputText("Search", findNodeTypeName.data(), findNodeTypeName.size() + 1, ImGuiInputTextFlags_CallbackResize,
					 [](ImGuiInputTextCallbackData* data) -> int
					 {
						 std::string* str = (std::string*)data->UserData;
						 str->resize(data->BufTextLen);
						 memcpy(&(*str)[0], data->Buf, data->BufTextLen);
						 return 0;
					 }, & findNodeTypeName);
	nodeTypes.clear();
	myGrid->GetNodeFactory().GetTypeNames(nodeTypes, findNodeTypeName);
	for (auto& item : nodeTypes)
	{
		ImGui::BeginChild(item.c_str(), ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoMove);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("NodeType", item.c_str(), item.size() + 1);
			ImGui::Text(item.c_str());
			ImGui::EndDragDropSource();
		}

		ImGui::Text(item.c_str());
		ImGui::EndChild();

	}

	ImGui::EndChild();




	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S, false))
	{
		Save();
	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_C, false))
	{
		Copy();
	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_V, false))
	{
		Paste();
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu((char*)u8"파일", true))
		{
			if (ImGui::MenuItem((char*)u8"저장", "Ctrl + S", nullptr, true))
			{
				Save();
			}

			if (ImGui::MenuItem((char*)u8"다른이름으로 저장", "Ctrl + shift + S", nullptr, true))
			{
				auto tempPath = WinUtility::GetSaveAsFilePath(L"Proj");

				Save(tempPath);
				Load(tempPath);
			}

			if (ImGui::MenuItem((char*)u8"열기", nullptr, nullptr, true))
			{
				auto newpath = WinUtility::GetOpenFilePath(L"Proj");
				Load(newpath, true);
			}

			if (ImGui::MenuItem((char*)u8"내보내기", nullptr, nullptr, true))
			{
				Export();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();

}

void NodeEditor::Save(std::filesystem::path path)
{
	if (path.empty())
	{
		path = this->path;
	}
	myGrid->path = path;
	if (!myGrid)
	{
		return;
	}

	nlohmann::json j;

	j["shadingModel"] = myGrid->shadingModel;
	j["blendMode"] = myGrid->blendMode;
	j["isParticle"] = isParticle;
	for (auto& i : myGrid->customData)
	{
		nlohmann::json shaderCustomData;
		shaderCustomData["name"] = i.name;
		shaderCustomData["type"] = i.type;
		shaderCustomData["guid"] = i.guid;
		j["ShaderCustomData"].push_back(shaderCustomData);
	}


	for (ImFlow::BaseNode* item : myGrid->getNodes() | std::views::transform([](const auto& item) {return item.second.get();}))
	{
		nlohmann::json nodeJson;

		auto typeview =
			std::string_view(typeid(*item).name())
			| std::views::reverse
			| std::views::take_while([](char item) { return item != ' '; })
			| std::views::reverse
			| std::views::common;

		std::string type(typeview.begin(), typeview.end());

		nodeJson["Type"] = type;
		nodeJson["pos"] = { item->getPos().x, item->getPos().y };
		nodeJson["uid"] = item->getUID();

		for (auto& pin : item->getIns())
		{
			nodeJson["pins"].push_back(pin->getUid());
		}
		for (auto& pin : item->getOuts())
		{
			nodeJson["pins"].push_back(pin->getUid());
		}

		ISerializable* serializeable = dynamic_cast<ISerializable*>(item);
		if (serializeable)
		{
			serializeable->Serialize(nodeJson);
		}


		j["nodes"].push_back(nodeJson);
	}

	for (const auto& item : myGrid->getLinks() | std::views::transform([](const auto& item) { return item.lock().get(); }))
	{
		if (!item) continue;
		nlohmann::json linkJson;
		
		linkJson["leftNode"] = item->left()->getParent()->getUID();
		linkJson["left"] = item->left()->getUid();

		linkJson["rightNode"] = item->right()->getParent()->getUID();
		linkJson["right"] = item->right()->getUid();
		
		j["links"].push_back(linkJson);
	}

	std::ofstream file(path);
	file << j.dump(4);
	file.close();


	myGrid->path = this->path;
}

void NodeEditor::Load(std::filesystem::path path, bool isSave)
{
	isPopUp = true;
	path.replace_extension(".Proj");
	if (isSave && std::filesystem::exists(this->path))
	{
		Save();
	}
	fileName = path.filename().string();
	myGrid = std::make_shared<NodeFlow>();
	this->path = path;
	myGrid->path = path;

	if (!std::filesystem::exists(this->path))
	{
		std::filesystem::create_directories(this->path.parent_path());
	}

	std::ifstream file(path);
	if (!file.is_open())
	{
		return;
	}
	nlohmann::json j;
	file >> j;
	file.close();
	if (j.find("shadingModel") != j.end())
	{
		myGrid->shadingModel = j["shadingModel"];
	}
	if (j.find("blendMode") != j.end())
	{
		myGrid->blendMode = j["blendMode"];
	}
	if (j.find("isParticle") != j.end())
	{
		isParticle = j["isParticle"];
	}
	if (j.find("ShaderCustomData") != j.end())
	{
		for (auto& item : j["ShaderCustomData"])
		{
			ShaderCustomData data;
			data.guid = item["guid"];
			data.name = item["name"];
			data.type = item["type"];
			myGrid->customData.push_back(data);
		}
	}

	std::map<ImFlow::NodeUID, std::shared_ptr<ImFlow::BaseNode>> nodes;
	for (auto& item : j["nodes"])
	{
		auto newNode = myGrid->Create(item["Type"]);
		newNode->setPos({ item["pos"][0], item["pos"][1] });

		ISerializable* serializeable = dynamic_cast<ISerializable*>(newNode.get());
		if (serializeable)
		{
			serializeable->Deserialize(item);
		}

		nodes.emplace(item["uid"], newNode);
	}

	for (auto& item : j["links"])
	{
		std::shared_ptr< ImFlow::Pin> left = nullptr;
		std::shared_ptr< ImFlow::Pin> right = nullptr;
		auto leftUid = item["left"].get<ImFlow::PinUID>();
		auto rightUid = item["right"].get<ImFlow::PinUID>();
		auto leftNodeUid = item["leftNode"].get<ImFlow::PinUID>();
		auto rightNodeUid = item["rightNode"].get<ImFlow::PinUID>();
		

		left = (nodes[leftNodeUid]->getOuts() | std::views::filter([&leftUid](auto& item) { return item->getUid() == leftUid; }) | std::views::take(1)).front();
		right = (nodes[rightNodeUid]->getIns() | std::views::filter([&rightUid](auto& item) { return item->getUid() == rightUid; }) | std::views::take(1)).front();

		if (left && right)
		{
			left->createLink(right.get());
		}
	}


}

void NodeEditor::Copy()
{
	nlohmann::json j;
	
	auto mouseGridPos = myGrid->screen2grid(ImGui::GetMousePos());
	j["mousePos.x"] = mouseGridPos.x;
	j["mousePos.y"] = mouseGridPos.y;

	for (ImFlow::BaseNode* item : myGrid->getNodes() | std::views::transform([](const auto& item) { return item.second.get(); }))
	{
		if (!item->isSelected()) continue;
		nlohmann::json nodeJson;


		auto typeview =
			std::string_view(typeid(*item).name())
			| std::views::reverse
			| std::views::take_while([](char item) { return item != ' '; })
			| std::views::reverse
			| std::views::common;

		std::string type(typeview.begin(), typeview.end());

		nodeJson["Type"] = type;
		nodeJson["pos"] = { item->getPos().x, item->getPos().y };
		nodeJson["uid"] = item->getUID();

		for (auto& pin : item->getIns())
		{
			nodeJson["pins"].push_back(pin->getUid());
		}
		for (auto& pin : item->getOuts())
		{
			nodeJson["pins"].push_back(pin->getUid());
		}

		ISerializable* serializeable = dynamic_cast<ISerializable*>(item);
		if (serializeable)
		{
			serializeable->Serialize(nodeJson);
		}


		j["nodes"].push_back(nodeJson);
	}

	for (const auto& item : myGrid->getLinks() | std::views::transform([](const auto& item) { return item.lock().get(); }))
	{
		if (!item) continue;
		if (!item->isSelected()) continue;
		nlohmann::json linkJson;

		linkJson["leftNode"] = item->left()->getParent()->getUID();
		linkJson["left"] = item->left()->getUid();

		linkJson["rightNode"] = item->right()->getParent()->getUID();
		linkJson["right"] = item->right()->getUid();

		j["links"].push_back(linkJson);
	}

	ImGui::SetClipboardText(j.dump().c_str());

}

void NodeEditor::Paste()
{
	const char* clipBoard = ImGui::GetClipboardText();
	nlohmann::json j;

	try
	{
		j = nlohmann::json::parse(clipBoard);
	}
	catch (nlohmann::json::parse_error& e)
	{
		std::cerr << "JSON 파싱 에러: " << e.what() << std::endl;
		return;
	}

	auto mousePos = ImVec2(j["mousePos.x"], j["mousePos.y"]);
	auto currentMousePos = myGrid->screen2grid(ImGui::GetMousePos());
	auto deltaMousePos = currentMousePos - mousePos;


	std::map<ImFlow::NodeUID, std::shared_ptr<ImFlow::BaseNode>> nodes;
	for (auto& item : j["nodes"])
	{
		auto newNode = myGrid->Create(item["Type"]);
		newNode->setPos(ImVec2{ item["pos"][0], item["pos"][1] } + deltaMousePos);

		ISerializable* serializeable = dynamic_cast<ISerializable*>(newNode.get());
		if (serializeable)
		{
			serializeable->Deserialize(item);
		}

		nodes.emplace(item["uid"], newNode);
	}

	for (auto& item : j["links"])
	{
		std::shared_ptr< ImFlow::Pin> left = nullptr;
		std::shared_ptr< ImFlow::Pin> right = nullptr;
		auto leftUid = item["left"].get<ImFlow::PinUID>();
		auto rightUid = item["right"].get<ImFlow::PinUID>();
		auto leftNodeUid = item["leftNode"].get<ImFlow::PinUID>();
		auto rightNodeUid = item["rightNode"].get<ImFlow::PinUID>();


		left = (nodes[leftNodeUid]->getOuts() | std::views::filter([&leftUid](auto& item) { return item->getUid() == leftUid; }) | std::views::take(1)).front();
		right = (nodes[rightNodeUid]->getIns() | std::views::filter([&rightUid](auto& item) { return item->getUid() == rightUid; }) | std::views::take(1)).front();

		if (left && right)
		{
			left->createLink(right.get());
		}
	}

}

void ShaderNodeEditor::UpdateImp()
{
	bool isMouseClicked = ImGui::IsMouseClicked(0);  // 좌클릭 확인
	if (isMouseClicked && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
	{
		if (ImGui::IsKeyDown(ImGuiKey_1))
		{
			myGrid->Create("ConstantValueNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_2))
		{
			myGrid->Create("ConstantVector2Node");
		}
		if (ImGui::IsKeyDown(ImGuiKey_3))
		{
			myGrid->Create("ConstantVector3Node");
		}
		if (ImGui::IsKeyDown(ImGuiKey_4))
		{
			myGrid->Create("ConstantVector4Node");
		}
		if (ImGui::IsKeyDown(ImGuiKey_T))
		{
			myGrid->Create("TextureNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_A))
		{
			myGrid->Create("AddNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_S))
		{
			myGrid->Create("SubNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_M))
		{
			myGrid->Create("MulNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_D))
		{
			myGrid->Create("DivNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_U))
		{
			myGrid->Create("TexCoordNode");
		}
	}

};


void ShaderNodeEditor::Load(std::filesystem::path path, bool isSave)
{
	if (path.empty()) return;
	NodeEditor::Load(path, isSave);



	myGrid->rightClickPopUpContent(
		[this](ImFlow::BaseNode* node)
		{
			if (!node)
			{
				if (ImGui::MenuItem((char*)u8"시간"))
				{
					myGrid->Create("TimeNode");
				}
				if (ImGui::MenuItem((char*)u8"float", "(1)"))
				{
					myGrid->Create("ConstantValueNode");
				}
				if (ImGui::MenuItem((char*)u8"float2", "(2)"))
				{
					myGrid->Create("ConstantVector2Node");
				}
				if (ImGui::MenuItem((char*)u8"float3", "(3)"))
				{
					myGrid->Create("ConstantVector3Node");
				}
				if (ImGui::MenuItem((char*)u8"float4", "(4)"))
				{
					myGrid->Create("ConstantVector4Node");
				}
				if (ImGui::MenuItem((char*)u8"텍스처", "(T)"))
				{
					myGrid->Create("TextureNode");
				}
				if (ImGui::MenuItem((char*)u8"더하기", "(A)"))
				{
					myGrid->Create("AddNode");
				}
				if (ImGui::MenuItem((char*)u8"빼기", "(S)"))
				{
					myGrid->Create("SubNode");
				}
				if (ImGui::MenuItem((char*)u8"곱하기", "(M)"))
				{
					myGrid->Create("MulNode");
				}
				if (ImGui::MenuItem((char*)u8"나누기", "(D)"))
				{
					myGrid->Create("DivNode");
				}
				if (ImGui::MenuItem((char*)u8"텍스처 좌표", "(U)"))
				{
					myGrid->Create("TexCoordNode");
				}
				if (ImGui::MenuItem((char*)u8"float2만들기"))
				{
					myGrid->Create("MakeVector2Node");
				}
				if (ImGui::MenuItem((char*)u8"float3만들기"))
				{
					myGrid->Create("MakeVector3Node");
				}
				if (ImGui::MenuItem((char*)u8"float4만들기"))
				{
					myGrid->Create("MakeVector4Node");
				}
				if (ImGui::MenuItem((char*)u8"float2분해"))
				{
					myGrid->Create("BreakVector2Node");
				}
				if (ImGui::MenuItem((char*)u8"float3분해"))
				{
					myGrid->Create("BreakVector3Node");
				}
				if (ImGui::MenuItem((char*)u8"float4분해"))
				{
					myGrid->Create("BreakVector4Node");
				}
			}
		});


	myGrid->droppedLinkPopUpContent(
		[this](ImFlow::Pin* dragged)
		{
			std::shared_ptr<ShaderNode> result;


			if (ImGui::MenuItem((char*)u8"더하기", "(A)"))
			{
				result = myGrid->Create("AddNode");
			}
			if (ImGui::MenuItem((char*)u8"빼기", "(S)"))
			{
				result = myGrid->Create("SubNode");
			}
			if (ImGui::MenuItem((char*)u8"곱하기", "(M)"))
			{
				result = myGrid->Create("MulNode");
			}
			if (ImGui::MenuItem((char*)u8"나누기", "(D)"))
			{
				result = myGrid->Create("DivNode");
			}
			if (ImGui::MenuItem((char*)u8"float2만들기"))
			{
				result = myGrid->Create("MakeVector2Node");
			}
			if (ImGui::MenuItem((char*)u8"float3만들기"))
			{
				result = myGrid->Create("MakeVector3Node");
			}
			if (ImGui::MenuItem((char*)u8"float4만들기"))
			{
				result = myGrid->Create("MakeVector4Node");
			}
			if (ImGui::MenuItem((char*)u8"float2분해"))
			{
				result = myGrid->Create("BreakVector2Node");
			}
			if (ImGui::MenuItem((char*)u8"float3분해"))
			{
				result = myGrid->Create("BreakVector3Node");
			}
			if (ImGui::MenuItem((char*)u8"float4분해"))
			{
				result = myGrid->Create("BreakVector4Node");
			}
			if (result && dragged->getType() == ImFlow::PinType::PinType_Output)
			{
				result->getIns().front()->createLink(dragged);
			}
		});


}

void ShaderNodeEditor::Export()
{
	Export(L"");
}

void ShaderNodeEditor::Export(const std::wstring& exportPath, bool Override)
{
	std::vector<std::shared_ptr<ShaderDataProcess>> originalNodeReturn;
	std::vector<std::shared_ptr<ShaderDataProcess>>& processVector = myGrid->GetShaderNodeReturn().data;
	processVector.clear();



	for (size_t i = 0; i < EShaderResult::MAX; i++)
	{
		auto value = myGrid->GetResultNode()->getInVal<ShaderPin<void>>(EShaderResult::pinNames[i]);

		for (auto& item : processVector)
		{
			originalNodeReturn.emplace_back(item);
		}

		if (value.value)
		{
			auto execution = std::make_shared<Execution>();
			execution->leftIdentifier = EShaderResult::hlslName[i];
			execution->rightIdentifier = value.value->identifier;
			originalNodeReturn.emplace_back(execution);
		}

		processVector.clear();
	}

	auto execution = std::make_shared<Execution>();
	execution->leftIdentifier = "material.ShadingModelID";
	execution->rightIdentifier = std::to_string(myGrid->shadingModel);
	originalNodeReturn.emplace_back(execution);

	bool isFoward = false;
	if (isParticle)
	{
		isFoward = true;
		auto define = std::make_shared<Define>();
		define->name = "PARTICLE";
		define->initializationExpression = "";
		originalNodeReturn.emplace_back(define);
	}
	switch (myGrid->shadingModel)
	{
	case EShadingModel::UI:
	{
		auto define = std::make_shared<Define>();
		define->name = "UI";
		define->initializationExpression = "";
		originalNodeReturn.emplace_back(define);

		isFoward = true;
		break;
	}
	default:
		break;
	}


	switch (myGrid->blendMode)
	{
	case EBlendMode::AlphaBlend:
	{
		auto define = std::make_shared<Define>();
		define->name = "BLEND_ALPHA";
		define->initializationExpression = "";
		originalNodeReturn.emplace_back(define);

		isFoward = true;
		break;
	}
	case EBlendMode::AlphaToCoverage:
	{
		auto define = std::make_shared<Define>();
		define->name = "ALPHA_TEST";
		define->initializationExpression = "";
		originalNodeReturn.emplace_back(define);

		break;
	}
	case EBlendMode::Dithering:
	{
		auto define = std::make_shared<Define>();
		define->name = "DITHERING";
		define->initializationExpression = "";
		originalNodeReturn.emplace_back(define);

		break;
	}
	case EBlendMode::Opaque:
	{
		break;
	}
	default:
		break;
	}

	if (isFoward)
	{
		auto define = std::make_shared<Define>();
		define->name = "FORWARD";
		define->initializationExpression = "";
		originalNodeReturn.emplace_back(define);
	}

	std::vector<Define*> defines;
	std::vector<CustomVariable*> customVariable;
	std::vector<RegistorVariable*> registerValues;
	std::vector<LocalVariable*> localValues;
	std::vector<Execution*> executions;

	std::set<std::shared_ptr<CustomVariable>> uniqueCustomVariableSet;
	std::set<std::string> uniqueSet;
	std::set<std::shared_ptr<LocalVariable>> uniqueSet2;
	std::set<std::shared_ptr<Execution>> uniqueSet3;

	for (auto& item : originalNodeReturn)
	{
		if (auto data = std::dynamic_pointer_cast<Define>(item))
		{
			defines.emplace_back(data.get());
		}
		else if (auto data = std::dynamic_pointer_cast<CustomVariable>(item))
		{
			if (uniqueCustomVariableSet.insert(data).second)
			{
				customVariable.emplace_back(data.get());
			}
		}
		else if (auto data = std::dynamic_pointer_cast<RegistorVariable>(item))
		{
			if (uniqueSet.insert(data->identifier).second)
			{
				registerValues.emplace_back(data.get());
			}
		}
		else if (auto data = std::dynamic_pointer_cast<LocalVariable>(item))
		{
			if (uniqueSet2.insert(data).second)
			{
				localValues.emplace_back(data.get());
			}
		}
		else if (auto data = std::dynamic_pointer_cast<Execution>(item))
		{
			if (uniqueSet3.insert(data).second)
			{
				executions.emplace_back(data.get());
			}
		}

	}

	std::stringstream defineLine;
	std::stringstream customValueLine;
	std::stringstream registerValueLine;
	std::stringstream localValueLine;
	std::stringstream executionsLine;

	for (auto& item : defines)
	{
		defineLine << *item << std::endl;
	}

	for (auto& item : customVariable)
	{
		customValueLine << *item << std::endl;
	}

	int registerCounts[ERegisterSlot::MAX] = { 0, };

	for (auto& item : registerValues)
	{
		item->slotNum = registerCounts[item->registorSlot]++;
		registerValueLine << *item << std::endl;
	}

	for (auto& item : localValues)
	{
		localValueLine << *item << std::endl;
	}

	for (auto& item : executions)
	{
		executionsLine << *item << std::endl;
	}



	std::string content = std::format(
		R"aa(
#include "Shared.hlsli"
#include "GBufferMaterial.hlsli"


struct CustomBuffer
{{
{0}
// 인풋 따로 받게끔 ex) 색, 가중치 같은거
}};

cbuffer CustomBuffer : register(b5)
{{
	CustomBuffer customData;
}};

{1}
{2}


GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

{3}
{4}
    return material;
}}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
)aa",
customValueLine.str(),
defineLine.str(),
registerValueLine.str(),
localValueLine.str(),
executionsLine.str()
);




	MaterialAsset materialAsset;
	materialAsset.OpenAsset(L"MaterialAssetTemp/NodeMaterialAsset.MaterialAsset");
	if (!materialAsset.SetPixelShader(content, isFoward))
	{
		return;
	}

	for (auto& item : registerValues)
	{
		std::filesystem::path relativePath = std::filesystem::relative(item->path, std::filesystem::current_path());

		materialAsset.SetTexture2D(relativePath.c_str(), item->slotNum);
	}

	if (exportPath.empty())
		materialAsset.SaveAsAssetWithDialog();
	else
		materialAsset.SaveAsAsset(exportPath.c_str(), Override);

	//MeshRender::ReloadShaderAll();

	originalNodeReturn.clear();
}
