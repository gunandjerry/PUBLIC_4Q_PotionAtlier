#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>
#include <fstream>
#include <filesystem>
#include "shaderNodes.h"
class NodeFlow;

class NodeEditor
{
public:
	NodeEditor();
	NodeEditor(const NodeEditor&) = delete;
	NodeEditor& operator=(const NodeEditor&) = delete;
	NodeEditor(NodeEditor&&) = delete;
	NodeEditor& operator=(NodeEditor&&) = delete;
	virtual ~NodeEditor();

	void Update();
	void Save(std::filesystem::path path = "");
	virtual void Load(std::filesystem::path path, bool isSave = false);
	virtual void Export() {};

	virtual void UpdateImp() {};
	void Copy();
	void Paste();

	std::vector<std::function<void()>> tickEvents;
	std::function<void()> EndPopupEvent;
protected:
	std::shared_ptr<NodeFlow> myGrid;
	std::shared_ptr<class NodeEditorManager> manager;
	friend class NodeEditorManager;
	std::filesystem::path path;
	std::string fileName;

	std::vector<std::string> nodeTypes;
	std::string findNodeTypeName;

	ImVec2 dragStartPos = ImVec2(0, 0);  // 드래그 시작 위치
	ImVec2 dragEndPos = ImVec2(0, 0);    // 드래그 끝 위치
	bool isDragging = false;              // 드래그 상태 확인
	bool isPopUp{ true };
	bool willSave{ true };
	bool isParticle{ false };
};

//unique_ptr 사용 권장
class ShaderNodeEditor : public NodeEditor
{
public:
	ShaderNodeEditor(std::filesystem::path path = "NodeEditor") : NodeEditor() 
	{
		Load(path);
	}

public:
	virtual void Load(std::filesystem::path path, bool isSave = false) override;
	virtual void Export() override;
	void Export(const std::wstring& exportPath, bool Override = false);

	virtual void UpdateImp() override;

	template<typename NodeType, typename ValueType>
	void SetResultNode(ValueType value, std::string_view pinName, EShaderResult::Type resultType)
	{
		auto newNode = myGrid->addNode<NodeType>({ 0,0 });
		newNode->Set(value);
		auto resultPin = myGrid->GetResultNode()->inPin(EShaderResult::pinNames[resultType]);
		auto outpin = newNode->outPin(pinName.data());
		outpin->createLink(resultPin);
	}

};

