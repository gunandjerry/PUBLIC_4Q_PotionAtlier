#include "NodeFactory.h"
#include "ShaderNodes.h"
#include <algorithm>

#define ADD_FACTORY_NODE(type)										\
nodeCreateFuncs[#type] = 											\
[](ImFlow::ImNodeFlow* nodeFlow) -> std::shared_ptr<ShaderNode>		\
	{ 																\
		return std::dynamic_pointer_cast<ShaderNode>(nodeFlow->placeNode<type>()); 						\
	}																\

ShaderNodeFactory::ShaderNodeFactory()
{
	ADD_FACTORY_NODE(ConstantValueNode);
	ADD_FACTORY_NODE(ConstantVector2Node);
	ADD_FACTORY_NODE(ConstantVector3Node);
	ADD_FACTORY_NODE(ConstantVector4Node);
	ADD_FACTORY_NODE(TextureNode);
	ADD_FACTORY_NODE(AddNode);
	ADD_FACTORY_NODE(SubNode);
	ADD_FACTORY_NODE(MulNode);
	ADD_FACTORY_NODE(DivNode);
	ADD_FACTORY_NODE(SinNode);
	ADD_FACTORY_NODE(CosNode);
	ADD_FACTORY_NODE(LerpNode);
	ADD_FACTORY_NODE(DotNode);
	ADD_FACTORY_NODE(CrossNode);
	ADD_FACTORY_NODE(NormalizeNode);
	ADD_FACTORY_NODE(LengthNode);
	ADD_FACTORY_NODE(SaturateNode);
	ADD_FACTORY_NODE(PowerNode);
	ADD_FACTORY_NODE(SqrtNode);
	ADD_FACTORY_NODE(AbsNode);
	ADD_FACTORY_NODE(UnPackNormalNode);
	ADD_FACTORY_NODE(TimeNode);
	ADD_FACTORY_NODE(TexCoordNode);
	ADD_FACTORY_NODE(MakeVector2Node);
	ADD_FACTORY_NODE(MakeVector3Node);
	ADD_FACTORY_NODE(MakeVector4Node);
	ADD_FACTORY_NODE(CustomValueNode);
	ADD_FACTORY_NODE(BreakVector2Node);
	ADD_FACTORY_NODE(BreakVector3Node);
	ADD_FACTORY_NODE(BreakVector4Node);
	ADD_FACTORY_NODE(FlipBook);
	ADD_FACTORY_NODE(FlipBookInterpolated);
	ADD_FACTORY_NODE(FmodeNode); 
	ADD_FACTORY_NODE(ParticleSurvivalTimeNode);
	ADD_FACTORY_NODE(RGBtoSRGBNode);
	ADD_FACTORY_NODE(SRGBtoRGBNode);
	ADD_FACTORY_NODE(RGBtoHSVNode);
	ADD_FACTORY_NODE(HSVtoRGBNode);
	
	nodeCreateFuncs["ShaderResultNode"] = 
		[](ImFlow::ImNodeFlow* nodeFlow) -> std::shared_ptr<ShaderNode> 
		{ 
			auto findNode = nodeFlow->getNodes()
				| std::views::transform([](const auto& item) { return std::dynamic_pointer_cast<ShaderNode>(item.second); })
				| std::views::filter([](const auto& item) { return !!std::dynamic_pointer_cast<ShaderResultNode>(item); });

			return *findNode.begin();

		};

}

ShaderNodeFactory::~ShaderNodeFactory() = default;

void ShaderNodeFactory::Set(NodeFlow* grid)
{
	myGrid = grid;
}

void ShaderNodeFactory::GetTypeNames(std::vector<std::string>& out, std::string findName)
{
	std::transform(findName.begin(), findName.end(), findName.begin(), [](unsigned char c) { return std::tolower(c); });
	std::string tempStr;
	auto temp =
		nodeCreateFuncs
		| std::views::transform([](const auto& item) -> std::string { return item.first; })
		| std::views::filter([findName, &tempStr](const auto& item)
							 {
								 tempStr.resize(item.size());
								 std::transform(item.begin(), item.end(), tempStr.begin(), [](unsigned char c) { return std::tolower(c); });
								 return tempStr.find(findName) != std::string::npos;
							 });

	std::ranges::copy(temp, std::back_inserter(out));

}

std::shared_ptr<ShaderNode> ShaderNodeFactory::Create(std::string_view type)
{
	assert(nodeCreateFuncs[type.data()]);
	return nodeCreateFuncs[type.data()](myGrid);
}
