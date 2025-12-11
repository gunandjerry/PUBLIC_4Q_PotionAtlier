#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>
#include <string>
#include <map>


class ShaderNode;
class NodeFlow;

class ShaderNodeFactory
{
public:
	ShaderNodeFactory();
	~ShaderNodeFactory();

public:
	void Set(NodeFlow* grid);
	std::shared_ptr<ShaderNode> Create(std::string_view type);
	template <typename T>
	std::shared_ptr<ShaderNode> Create(std::string_view type)
	{
		return std::dynamic_pointer_cast<T>(Create(type));
	}

	void GetTypeNames(std::vector<std::string>& out, std::string findName);


private:
	NodeFlow* myGrid;
	std::map<std::string, std::function<std::shared_ptr<ShaderNode>(ImFlow::ImNodeFlow*)>> nodeCreateFuncs;
};


