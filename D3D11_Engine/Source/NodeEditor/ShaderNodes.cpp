#include "ShaderNodes.h"
#include "Utility/WinUtility.h"
#include "Manager\TextureManager.h"

ShaderNode::ShaderNode()
{
	setStyle(ImFlow::NodeStyle::green());
	
}

NodeFlow* ShaderNode::GetHandler()
{
	return dynamic_cast<NodeFlow*>(getHandler());
}

void ShaderNode::UnLabelPinRenderer(ImFlow::Pin* p)
{
	ImGui::SetCursorPos(p->getPos());
	ImGui::Dummy({ 40, 20 });
	p->drawDecoration();
	p->drawSocket();
}

std::function<bool(ImFlow::Pin*, ImFlow::Pin*)> ShaderNode::SameType()
{
	return 
		[](ImFlow::Pin* out, ImFlow::Pin* in)
		{
			if (out->getDataType() == typeid(ShaderPin<void>) || in->getDataType() == typeid(ShaderPin<void>))
				return true;
			
			return out->getDataType() == in->getDataType();
		};
}

std::function<bool(ImFlow::Pin*, ImFlow::Pin*)> ShaderNode::SameTypeBrotherPin()
{
	return
		[](ImFlow::Pin* out, ImFlow::Pin* in)
		{
			for (auto& item  : in->getParent()->getIns())
			{
				if (item.get() != in)
				{
					if (item->isConnected())
					{
						if (out->getDataType() == typeid(ShaderPin<void>) || 
							item->getLink().lock()->left()->getDataType() == typeid(ShaderPin<void>))
							return true;
						return out->getDataType() == item->getLink().lock()->left()->getDataType();
					}
					else
					{
						return true;
					}
				}
			}

			return true;
		};
}


ConstantValueNode::ConstantValueNode()
{
	Set(0.0f);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderPin<float>>((char*)u8"값")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
}

void ConstantValueNode::Set(float value)
{
	setTitle(std::format("{}", value));
	this->value = value;
}

void ConstantValueNode::draw()
{
	ImGui::SetNextItemWidth(100);
	ImGui::PushID("x");
	if (ImGui::DragFloat("", &value, 0.01f))
	{
		Set(value);
	}
	ImGui::PopID();
}

void ConstantValueNode::Serialize(nlohmann::json& j)
{
	j["value"] = value;
}

void ConstantValueNode::Deserialize(const nlohmann::json& j)
{
	value = j["value"];
	Set(value);
}

ConstantVector2Node::ConstantVector2Node()
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderPin<Vector2>>((char*)u8"값")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float2";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float2({}, {})" }, value.x, value.y);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector2>{ var };
		});
	addOUT<ShaderPin<float>>("x")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.x);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("y")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.y);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
}
void ConstantVector2Node::Set(const Vector2& value)
{
	setTitle(std::format({ "{}, {}" }, value.x, value.y));
	this->value = value;
}

void ConstantVector2Node::draw()
{
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		if (ImGui::InputFloat("", &value.x))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		if (ImGui::InputFloat("", &value.y))
		{
			Set(value);
		}
		ImGui::PopID();
	}
}

void ConstantVector2Node::Serialize(nlohmann::json& j)
{
	j["value.x"] = value.x;
	j["value.y"] = value.y;
}

void ConstantVector2Node::Deserialize(const nlohmann::json& j)
{
	value.x = j["value.x"];
	value.y = j["value.y"];
	Set(value);
}

ConstantVector3Node::ConstantVector3Node()
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderPin<Vector3>>((char*)u8"값")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float3({}, {}, {})" }, value.x, value.y, value.z);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector3>{ var };
		});
	addOUT<ShaderPin<float>>("x")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.x);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("y")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.y);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("z")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_z{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.z);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
}

void ConstantVector3Node::Set(const Vector3& value)
{
	setTitle(std::format({ "{}, {}. {}" }, value.x, value.y, value.z));
	this->value = value;
}

void ConstantVector3Node::draw()
{
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		if (ImGui::InputFloat("", &value.x))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		if (ImGui::InputFloat("", &value.y))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("z");
		if (ImGui::InputFloat("", &value.z))
		{
			Set(value);
		}
		ImGui::PopID();
	}
}

void ConstantVector3Node::Serialize(nlohmann::json& j)
{
	j["value.x"] = value.x;
	j["value.y"] = value.y;
	j["value.z"] = value.z;
}

void ConstantVector3Node::Deserialize(const nlohmann::json& j)
{
	value.x = j["value.x"];
	value.y = j["value.y"];
	value.z = j["value.z"];
	Set(value);
}

ConstantVector4Node::ConstantVector4Node()
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderPin<Vector4>>((char*)u8"값")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float4";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float4({}, {}, {}, {})" }, value.x, value.y, value.z, value.w);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector4>{ var };
		});
	addOUT<ShaderPin<float>>("x")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.x);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("y")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.y);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("z")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_z{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.z);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("w")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_w{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.w);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
}

void ConstantVector4Node::Set(const Vector4& value)
{
	setTitle(std::format({ "{}, {}. {}, {}" }, value.x, value.y, value.z, value.w));
	this->value = value;
}

void ConstantVector4Node::draw()
{
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		if (ImGui::InputFloat("", &value.x))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		if (ImGui::InputFloat("", &value.y))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("z");
		if (ImGui::InputFloat("", &value.z))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("w");
		if (ImGui::InputFloat("", &value.w))
		{
			Set(value);
		}
		ImGui::PopID();
	}
}

void ConstantVector4Node::Serialize(nlohmann::json& j)
{
	j["value.x"] = value.x;
	j["value.y"] = value.y;
	j["value.z"] = value.z;
	j["value.w"] = value.w;
}

void ConstantVector4Node::Deserialize(const nlohmann::json& j)
{
	value.x = j["value.x"];
	value.y = j["value.y"];
	value.z = j["value.z"];
	value.w = j["value.w"];
	Set(value);
}

static constexpr const char* samplerName[2] = { "DefaultSampler", "ClampSampler" };
TextureNode::TextureNode()
{
	setTitle((char*)u8"텍스처");
	Set(texturePath);
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<void>>("UV", {}, 
						   [this](ImFlow::Pin* out, ImFlow::Pin* in) -> bool
						   {
							   return IsValidShaderPinType(out);
						   });

	addOUT<ShaderPin<Vector3>>((char*)u8"RGB")->behaviour(
		[this]()
		{
			std::string texcoord = "input.Tex";
			auto uvValue = getInVal<ShaderPin<void>>("UV");
			if (uvValue.value)
			{
				texcoord = uvValue.value->identifier;
			}
		

			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;
			var->path = texturePath;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float3";
			var2->identifier = std::format({ "c_{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample({}, {}).rgb" }, var->identifier, samplerName[samplerIndex], texcoord);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			return ShaderPin<Vector3>{ var2 };
		});
	addOUT<ShaderPin<float>>((char*)u8"R")->behaviour(
		[this]()
		{
			std::string texcoord = "input.Tex";
			auto uvValue = getInVal<ShaderPin<void>>("UV");
			if (uvValue.value)
			{
				texcoord = uvValue.value->identifier;
			}

			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;
			var->path = texturePath;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float3";
			var2->identifier = std::format({ "c_R{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample({}, {}).r" }, var->identifier, samplerName[samplerIndex], texcoord);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			return ShaderPin<float>{ var2 };
		});
	addOUT<ShaderPin<float>>((char*)u8"G")->behaviour(
		[this]()
		{
			std::string texcoord = "input.Tex";
			auto uvValue = getInVal<ShaderPin<void>>("UV");
			if (uvValue.value)
			{
				texcoord = uvValue.value->identifier;
			}

			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;
			var->path = texturePath;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float3";
			var2->identifier = std::format({ "c_G{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample({}, {}).g" }, var->identifier, samplerName[samplerIndex], texcoord);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			return ShaderPin<float>{ var2 };
		});
	addOUT<ShaderPin<float>>((char*)u8"B")->behaviour(
		[this]()
		{
			std::string texcoord = "input.Tex";
			auto uvValue = getInVal<ShaderPin<void>>("UV");
			if (uvValue.value)
			{
				texcoord = uvValue.value->identifier;
			}

			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;
			var->path = texturePath;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float";
			var2->identifier = std::format({ "c_B{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample({}, {}).b" }, var->identifier, samplerName[samplerIndex], texcoord);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			return ShaderPin<float>{ var2 };
		});
	addOUT<ShaderPin<float>>((char*)u8"A")->behaviour(
		[this]()
		{
			std::string texcoord = "input.Tex";
			auto uvValue = getInVal<ShaderPin<void>>("UV");
			if (uvValue.value)
			{
				texcoord = uvValue.value->identifier;
			}

			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;
			var->path = texturePath;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float";
			var2->identifier = std::format({ "c_A{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample({}, {}).a" }, var->identifier, samplerName[samplerIndex], texcoord);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			return ShaderPin<float>{ var2 };
		});
	addOUT<ShaderPin<Vector4>>((char*)u8"RGBA")->behaviour(
		[this]()
		{
			std::string texcoord = "input.Tex";
			auto uvValue = getInVal<ShaderPin<void>>("UV");
			if (uvValue.value)
			{
				texcoord = uvValue.value->identifier;
			}

			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;
			var->path = texturePath;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float4";
			var2->identifier = std::format({ "c_RGBA{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample({}, {}).rgba" }, var->identifier, samplerName[samplerIndex], texcoord);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			return ShaderPin<Vector4>{ var2 };
		});

	addOUT<ShaderPin<void>>((char*)u8"텍스처")->behaviour(
		[this]()
		{
			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;
			var->path = texturePath;

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		});


}

TextureNode::~TextureNode()
{
	if ((ID3D11ShaderResourceView*)texture)
	{
		texture.~Texture();
		//textureManager.ReleaseSharingTexture(texturePath.c_str());
	}
}

void TextureNode::Set(const std::filesystem::path& value)
{
	if (value.empty()) return;
	if (!std::filesystem::exists(value))
	{
		this->destroy();
		return;
	}
	if ((ID3D11ShaderResourceView*)texture)
	{
		texture.~Texture();
		//textureManager.ReleaseSharingTexture(texturePath.c_str());
	}

	texturePath = value;

	std::unique_ptr<DirectX::ScratchImage> image = std::make_unique<DirectX::ScratchImage>();
	DirectX::TexMetadata metadata;
	HRESULT result;
	if (texturePath.extension() == ".dds")
	{
		result = DirectX::LoadFromDDSFile(texturePath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, *image);
	}
	else
	{
		result = DirectX::LoadFromWICFile(texturePath.c_str(), DirectX::WIC_FLAGS_IGNORE_SRGB, &metadata, *image);
	}
	Check(result);
	texture.CreateTexture(std::move(image), ETextureUsage::SRV);
	dimension = metadata.dimension;

	//ComPtr<ID3D11ShaderResourceView> srv;
	//textureManager.CreateSharingTexture(value.c_str(), &srv);
	//srv->AddRef();

	//texture.LoadTexture(srv.Get());
	//D3D11_SHADER_RESOURCE_VIEW_DESC textureDesc;
	//srv->GetDesc(&textureDesc);
	//dimension = textureDesc.ViewDimension;
	auto pin = this->inPin("UV");
	if (pin->isConnected() && !IsValidShaderPinType(pin->getLink().lock()->left()))
	{
		pin->deleteLink();
	}
}

void TextureNode::draw()
{
	if (ImGui::Button("Load Texture"))
	{
		auto path = WinUtility::GetOpenFilePath();
		if (!path.empty())
		{
			Set(path);
		}
	}
	//
	if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
	{
		ImGui::Image((ImTextureID)(ID3D11ShaderResourceView*)texture, { 100, 100 });
	}
	ImGui::PushItemWidth(200);
	ImGui::Combo("Sampler", &samplerIndex, samplerName, IM_ARRAYSIZE(samplerName));
	ImGui::PopItemWidth();

}

void TextureNode::Serialize(nlohmann::json& j)
{
	std::filesystem::path projPath = GetHandler()->path;
	std::filesystem::path currPath = std::filesystem::current_path();

	if (projPath.is_relative())
	{
		projPath = currPath / projPath.parent_path();
	}

	std::filesystem::path relativePath = std::filesystem::relative(texturePath, projPath);
	j["path"] = relativePath.string();	
	j["sampler"] = samplerIndex;
}

void TextureNode::Deserialize(const nlohmann::json& j)
{
	if (j.find("path") != j.cend())
	{
		std::string pathStr = j["path"].get<std::string>();
		if (pathStr.empty())
		{
			return;
		}
		std::filesystem::path projPath = GetHandler()->path;
		std::filesystem::path currPath = std::filesystem::current_path();

		if (projPath.is_relative())
		{
			projPath = currPath / projPath.parent_path();
		}

		texturePath = projPath / std::filesystem::path(pathStr);
		Set(texturePath);
	}
	if (j.find("sampler") != j.cend())
	{
		samplerIndex = j["sampler"];
	}
}

bool TextureNode::IsValidShaderPinType(ImFlow::Pin* out)
{
	if (out->getDataType() == typeid(ShaderPin<void>))
	{
		return true;
	}
	switch (dimension)
	{
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		return out->getDataType() == typeid(ShaderPin<float>);
	case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		return out->getDataType() == typeid(ShaderPin<Vector2>);
	case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		return out->getDataType() == typeid(ShaderPin<Vector3>);
	default:
		return false;
	}
}


ShaderResultNode::ShaderResultNode()
{
	setStyle(ImFlow::NodeStyle::red());
	setTitle("BRDF");

	addIN<ShaderPin<Vector3>>(EShaderResult::pinNames[EShaderResult::Albedo], {}, SameType());
	addIN<ShaderPin<Vector3>>(EShaderResult::pinNames[EShaderResult::Normal], {}, SameType());
	addIN<ShaderPin<float>>(EShaderResult::pinNames[EShaderResult::Metallic], {}, SameType());
	addIN<ShaderPin<float>>(EShaderResult::pinNames[EShaderResult::Roughness], {}, SameType());
	addIN<ShaderPin<float>>(EShaderResult::pinNames[EShaderResult::Alpha], {}, SameType());
	addIN<ShaderPin<Vector3>>(EShaderResult::pinNames[EShaderResult::Specular], {}, SameType());
	addIN<ShaderPin<float>>(EShaderResult::pinNames[EShaderResult::EnvironmentOcclusion], {}, SameType());
	addIN<ShaderPin<Vector3>>(EShaderResult::pinNames[EShaderResult::Emissive], {}, SameType());
	addIN<ShaderPin<Vector4>>(EShaderResult::pinNames[EShaderResult::GBuffer0], {}, SameType());
	addIN<ShaderPin<Vector4>>(EShaderResult::pinNames[EShaderResult::GBuffer1], {}, SameType());
	addIN<ShaderPin<Vector4>>(EShaderResult::pinNames[EShaderResult::GBuffer2], {}, SameType());
	addIN<ShaderPin<Vector4>>(EShaderResult::pinNames[EShaderResult::GBuffer3], {}, SameType());

}

const char* TimeNode::timeName[2] = { "Time", "Time0_1" };
TimeNode::TimeNode()
{
	setTitle((char*)u8"시간");
	setStyle(ImFlow::NodeStyle::cyan());
	addOUT<ShaderPin<float>>("")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "frameData.{}" }, timeName[currItem]);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
}

void TimeNode::draw()
{
	//사이즈	조절
	ImGui::PushItemWidth(120);
	ImGui::PushID("combbo");
	ImGui::Combo("", &currItem, timeName, IM_ARRAYSIZE(timeName));
	ImGui::PopID();
	ImGui::PopItemWidth();
	
}

void TimeNode::Serialize(nlohmann::json& j)
{
	j["currItem"] = currItem;
}

void TimeNode::Deserialize(const nlohmann::json& j)
{
	if (j.find("currItem") != j.cend())
	{
		currItem = j["currItem"];
	}
}


AddNode::AddNode()
{
	setTitle((char*)u8"더하기");
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<void>>("a", {}, SameTypeBrotherPin())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<void>>("b", {}, SameTypeBrotherPin())->renderer(UnLabelPinRenderer);

	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto b = getInVal<ShaderPin<void>>("b");

			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "{} + {}" }, a.value->identifier, b.value->identifier);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);
}

void AddNode::draw()
{
	ImGui::Dummy(ImGui::GetItemRectSize() * 0.25f);
	ImGui::Text(" + ");
}

SubNode::SubNode()
{
	setTitle((char*)u8"빼기");
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<void>>("a", {}, SameTypeBrotherPin())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<void>>("b", {}, SameTypeBrotherPin())->renderer(UnLabelPinRenderer);

	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto b = getInVal<ShaderPin<void>>("b");

			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "{} - {}" }, a.value->identifier, b.value->identifier);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);
}

void SubNode::draw()
{
	ImGui::Dummy(ImGui::GetItemRectSize() * 0.25f);
	ImGui::Text(" - ");
}

MulNode::MulNode()
{
	setTitle((char*)u8"곱하기");
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<void>>("b", {}, SameType())->renderer(UnLabelPinRenderer);

	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto b = getInVal<ShaderPin<void>>("b");

			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "{} * {}" }, a.value->identifier, b.value->identifier);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);
}

void MulNode::draw()
{
	ImGui::Dummy(ImGui::GetItemRectSize() * 0.25f);
	ImGui::Text(" X ");
}

DivNode::DivNode()
{
	setTitle((char*)u8"나누기");
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<void>>("b", {}, SameType())->renderer(UnLabelPinRenderer);

	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto b = getInVal<ShaderPin<void>>("b");

			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "{} / {}" }, a.value->identifier, b.value->identifier);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);
}

void DivNode::draw()
{
	ImGui::Dummy(ImGui::GetItemRectSize() * 0.25f);
	ImGui::Text(" / ");
}

LerpNode::LerpNode()
{
	setTitle((char*)u8"선형보간");
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<void>>("x", {}, SameType())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<void>>("y", {}, SameType())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<float>>("a", {}, SameType())->renderer(UnLabelPinRenderer);

	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("x");
			auto b = getInVal<ShaderPin<void>>("y");
			auto c = getInVal<ShaderPin<float>>("a");

			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "	lerp({}, {}, {})" }, a.value->identifier, b.value->identifier, c.value->identifier);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);
}

DotNode::DotNode()
{
	setTitle((char*)u8"내적");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<void>>("b", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<float>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto b = getInVal<ShaderPin<void>>("b");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "dot({}, {})" }, a.value->identifier, b.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		})->renderer(UnLabelPinRenderer);;
}

CrossNode::CrossNode()
{
	setTitle((char*)u8"외적");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<void>>("b", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto b = getInVal<ShaderPin<void>>("b");
			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "cross({}, {})" }, a.value->identifier, b.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);;
}

NormalizeNode::NormalizeNode()
{
	setTitle((char*)u8"정규화");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "normalize({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);;
}

LengthNode::LengthNode()
{
	setTitle((char*)u8"길이");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<float>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "length({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		})->renderer(UnLabelPinRenderer);;
}

SaturateNode::SaturateNode()
{
	setTitle((char*)u8"Saturate");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "saturate({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);;
}

PowerNode::PowerNode()
{
	setTitle((char*)u8"제곱");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<float>>("b", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto b = getInVal<ShaderPin<float>>("b");
			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "pow({}, {})" }, a.value->identifier, b.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);;
}

SqrtNode::SqrtNode()
{
	setTitle((char*)u8"제곱근");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "sqrt({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);;
}

AbsNode::AbsNode()
{
	setTitle((char*)u8"절대값");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<void>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<void>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "abs({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		})->renderer(UnLabelPinRenderer);;
}

UnPackNormalNode::UnPackNormalNode()
{
	setTitle((char*)u8"법선복원");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<Vector3>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<Vector3>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<Vector3>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = a.value->type;
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "{} * 2.0 - 1.0" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector3>{ var };
		})->renderer(UnLabelPinRenderer);;
}



TexCoordNode::TexCoordNode()
{
	setTitle((char*)u8"uv셋 0");
	addOUT<ShaderPin<void>>("uv(0)")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float2";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = "input.Tex";

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<void>{ var };
		});
}


NodeFlow::NodeFlow()
{
	nodeFactory.Set(this);
	resultNode = addNode<ShaderResultNode>({ 100, 100 }).get();
}

NodeFlow::~NodeFlow() = default;

BreakVector2Node::BreakVector2Node()
{
	setTitle((char*)u8"벡터2 분해");
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<Vector2>>("", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<float>>("x")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}.x" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("y")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}.y" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
}
BreakVector3Node::BreakVector3Node()
{
	setTitle((char*)u8"벡터3 분해");
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<Vector3>>("", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<float>>("x")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}.x" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("y")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}.y" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("z")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_z{}" }, getUID());
			var->initializationExpression = std::format({ "{}.z" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
}

BreakVector4Node::BreakVector4Node()
{
	setTitle((char*)u8"벡터4 분해");
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin<Vector4>>("", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<float>>("x")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}.x" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("y")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}.y" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("z")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_z{}" }, getUID());
			var->initializationExpression = std::format({ "{}.z" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
	addOUT<ShaderPin<float>>("w")->behaviour(
		[this]()
		{
			auto vector = getInVal<ShaderPin<Vector2>>("");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_w{}" }, getUID());
			var->initializationExpression = std::format({ "{}.w" }, vector.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		});
}


MakeVector2Node::MakeVector2Node()
{
	setTitle((char*)u8"벡터2");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<float>>("x", {}, SameType());
	addIN<ShaderPin<float>>("y", {}, SameType());
	addOUT<ShaderPin<Vector2>>("")->behaviour(
		[this]()
		{
			auto xPin = getInVal<ShaderPin<float>>("x");
			auto yPin = getInVal<ShaderPin<float>>("y");

			std::string xIdentifier = std::to_string(x);
			std::string yIdentifier = std::to_string(y);

			if (xPin.value)
			{
				xIdentifier = xPin.value->identifier;
			}			
			if (yPin.value)
			{
				yIdentifier = yPin.value->identifier;
			}

			auto var = std::make_shared<LocalVariable>();
			var->type = "float2";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float2({}, {})" }, xIdentifier, yIdentifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector2>{ var };
		})->renderer(UnLabelPinRenderer);
}

void MakeVector2Node::draw()
{
	if (inPin("x")->isConnected())
	{
		ImGui::Dummy({40, 20});
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		ImGui::DragFloat("", &x, 0.01f);
		ImGui::PopID();
	}

	if (inPin("y")->isConnected())
	{
		ImGui::Dummy({ 40, 20 });
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		ImGui::DragFloat("", &y, 0.01f);
		ImGui::PopID();
	}

}

void MakeVector2Node::Serialize(nlohmann::json& j)
{
	j["x"] = x;
	j["y"] = y;
}

void MakeVector2Node::Deserialize(const nlohmann::json& j)
{
	if (j.find("x") != j.cend())
	{
		x = j["x"];
	}
	if (j.find("y") != j.cend())
	{
		y = j["y"];
	}
}

MakeVector3Node::MakeVector3Node()
{
	setTitle((char*)u8"벡터3");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<float>>("x", {}, SameType());
	addIN<ShaderPin<float>>("y", {}, SameType());
	addIN<ShaderPin<float>>("z", {}, SameType());
	addOUT<ShaderPin<Vector3>>("")->behaviour(
		[this]()
		{
			auto xPin = getInVal<ShaderPin<float>>("x");
			auto yPin = getInVal<ShaderPin<float>>("y");
			auto zPin = getInVal<ShaderPin<float>>("z");

			std::string xIdentifier = std::to_string(x);
			std::string yIdentifier = std::to_string(y);
			std::string zIdentifier = std::to_string(z);
			if (xPin.value)
			{
				xIdentifier = xPin.value->identifier;
			}
			if (yPin.value)
			{
				yIdentifier = yPin.value->identifier;
			}
			if (zPin.value)
			{
				zIdentifier = zPin.value->identifier;
			}

			auto var = std::make_shared<LocalVariable>();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float3({}, {}, {})" }, xIdentifier, yIdentifier, zIdentifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector3>{ var };
		});
}

void MakeVector3Node::Serialize(nlohmann::json& j)
{
	j["x"] = x;
	j["y"] = y;
	j["z"] = z;
}

void MakeVector3Node::Deserialize(const nlohmann::json& j)
{
	if (j.find("x") != j.cend())
	{
		x = j["x"];
	}
	if (j.find("y") != j.cend())
	{
		y = j["y"];
	}
	if (j.find("z") != j.cend())
	{
		z = j["z"];
	}
}

void MakeVector3Node::draw()
{
	if (inPin("x")->isConnected())
	{
		ImGui::Dummy({ 40, 20 });
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		ImGui::DragFloat("", &x, 0.01f);
		ImGui::PopID();
	}
	if (inPin("y")->isConnected())
	{
		ImGui::Dummy({ 40, 20 });
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		ImGui::DragFloat("", &y, 0.01f);
		ImGui::PopID();
	}
	if (inPin("z")->isConnected())
	{
		ImGui::Dummy({ 40, 20 });
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("z");
		ImGui::DragFloat("", &z, 0.01f);
		ImGui::PopID();
	}
}

MakeVector4Node::MakeVector4Node()
{
	setTitle((char*)u8"벡터4");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<float>>("x", {}, SameType());
	addIN<ShaderPin<float>>("y", {}, SameType());
	addIN<ShaderPin<float>>("z", {}, SameType());
	addIN<ShaderPin<float>>("w", {}, SameType());
	addOUT<ShaderPin<Vector4>>("")->behaviour(
		[this]()
		{
			auto xPin = getInVal<ShaderPin<float>>("x");
			auto yPin = getInVal<ShaderPin<float>>("y");
			auto zPin = getInVal<ShaderPin<float>>("z");
			auto wPin = getInVal<ShaderPin<float>>("w");

			std::string xIdentifier = std::to_string(x);
			std::string yIdentifier = std::to_string(y);
			std::string zIdentifier = std::to_string(z);
			std::string wIdentifier = std::to_string(w);
			if (xPin.value)
			{
				xIdentifier = xPin.value->identifier;
			}
			if (yPin.value)
			{
				yIdentifier = yPin.value->identifier;
			}
			if (zPin.value)
			{
				zIdentifier = zPin.value->identifier;
			}
			if (wPin.value)
			{
				wIdentifier = wPin.value->identifier;
			}
			auto var = std::make_shared<LocalVariable>();
			var->type = "float4";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float4({}, {}, {}, {})" }, xIdentifier, yIdentifier, zIdentifier, wIdentifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector4>{ var };
		})->renderer(UnLabelPinRenderer);
}

void MakeVector4Node::Serialize(nlohmann::json& j)
{
	j["x"] = x;
	j["y"] = y;
	j["z"] = z;
	j["w"] = w;
}

void MakeVector4Node::Deserialize(const nlohmann::json& j)
{
	if (j.find("x") != j.cend())
	{
		x = j["x"];
	}
	if (j.find("y") != j.cend())
	{
		y = j["y"];
	}
	if (j.find("z") != j.cend())
	{
		z = j["z"];
	}
	if (j.find("w") != j.cend())
	{
		w = j["w"];
	}
}

void MakeVector4Node::draw()
{
	if (inPin("x")->isConnected())
	{
		ImGui::Dummy({ 40, 20 });
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		ImGui::DragFloat("", &x, 0.01f);
		ImGui::PopID();
	}
	if (inPin("y")->isConnected())
	{
		ImGui::Dummy({ 40, 20 });
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		ImGui::DragFloat("", &y, 0.01f);
		ImGui::PopID();
	}
	if (inPin("z")->isConnected())
	{
		ImGui::Dummy({ 40, 20 });
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("z");
		ImGui::DragFloat("", &z, 0.01f);
		ImGui::PopID();
	}
	if (inPin("w")->isConnected())
	{
		ImGui::Dummy({ 40, 20 });
	}
	else
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("w");
		ImGui::DragFloat("", &w, 0.01f);
		ImGui::PopID();
	}
}

CustomValueNode::CustomValueNode()
{
	setStyle(ImFlow::NodeStyle::cyan());
	addOUT<ShaderPin<void>>((char*)u8"값")->behaviour(
		[this]()
		{
			auto temp = GetHandler()->customData | std::views::filter([this](const auto& data) { return data.guid == guid; });
			ShaderCustomData& data = temp.front();

			auto var = std::make_shared<CustomVariable>();
			var->type = ECustomDataTyoe::name[data.type];
			var->identifier = std::format("{}", data.name) ;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = ECustomDataTyoe::name[data.type];
			var2->identifier = std::format("{}", data.name);
			var2->initializationExpression = std::format("customData.{}", var->identifier);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			return ShaderPin<void>{ var2 };
		});
}

void CustomValueNode::Set(size_t guid)
{
	this->guid = guid;
	auto temp = GetHandler()->customData | std::views::filter([guid](const auto& data) { return data.guid == guid; });
	ShaderCustomData& data = temp.front();
	setTitle(std::format({ "{} {}" }, ECustomDataTyoe::name[data.type], data.name));
}

void CustomValueNode::draw()
{
	auto temp = GetHandler()->customData | std::views::filter([this](const auto& data) { return data.guid == guid; });
	ShaderCustomData& data = temp.front();
	setTitle(std::format({ "{} {}" }, ECustomDataTyoe::name[data.type], data.name));

}

void CustomValueNode::Serialize(nlohmann::json& j)
{
	j["guid"] = guid;
}

void CustomValueNode::Deserialize(const nlohmann::json& j)
{
	if (j.find("guid") != j.cend())
	{
		guid = j["guid"];
		Set(guid);
	}
}

FlipBook::FlipBook()
{
	setTitle((char*)u8"플립북");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<float>>((const char*)u8"재생시간", {}, SameType());
	addIN<ShaderPin<float>>((const char*)u8"열", {}, SameType());
	addIN<ShaderPin<float>>((const char*)u8"행", {}, SameType());
	addOUT<ShaderPin<Vector2>>("UV")->behaviour(
		[this]()
		{
			auto time = getInVal<ShaderPin<float>>((const char*)u8"재생시간");
			auto cols = getInVal<ShaderPin<float>>((const char*)u8"열");
			auto rows = getInVal<ShaderPin<float>>((const char*)u8"행");

			auto flipBookVar = std::make_shared<LocalVariable>();
			flipBookVar->type = "float2";
			flipBookVar->identifier = std::format("uv_{}", getUID());
			flipBookVar->initializationExpression = std::format("FlipBook({}, {}, {}, input.Tex)",
																time.value->identifier,
																cols.value->identifier,
																rows.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(flipBookVar);

			return ShaderPin<Vector2>{ flipBookVar };
		});

}

FlipBookInterpolated::FlipBookInterpolated()
{
	setTitle((char*)u8"보간 플립북");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>((const char*)u8"텍스처", {}, SameType());
	addIN<ShaderPin<float>>((const char*)u8"재생시간", {}, SameType());
	addIN<ShaderPin<float>>((const char*)u8"열", {}, SameType());
	addIN<ShaderPin<float>>((const char*)u8"행", {}, SameType());
	addOUT<ShaderPin<Vector4>>("RGBA")->behaviour(
		[this]()
		{
			auto texture = getInVal<ShaderPin<float>>((const char*)u8"텍스처");
			auto time = getInVal<ShaderPin<float>>((const char*)u8"재생시간");
			auto cols = getInVal<ShaderPin<float>>((const char*)u8"열");
			auto rows = getInVal<ShaderPin<float>>((const char*)u8"행");

			auto flipBookVar = std::make_shared<LocalVariable>();
			flipBookVar->type = "float4";
			flipBookVar->identifier = std::format("color_{}", getUID());
			flipBookVar->initializationExpression = std::format("FlipBook({}, {}, {}, {}, input.Tex)",
																texture.value->identifier,
																time.value->identifier,
																cols.value->identifier,
																rows.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(flipBookVar);

			return ShaderPin<Vector4>{ flipBookVar };
		});

}

FmodeNode::FmodeNode()
{
	setTitle((char*)u8"Fmod");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<float>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addIN<ShaderPin<float>>("b", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<float>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<float>>("a");
			auto b = getInVal<ShaderPin<float>>("b");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "fmod({}, {})" }, a.value->identifier, b.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		})->renderer(UnLabelPinRenderer);
}

ParticleSurvivalTimeNode::ParticleSurvivalTimeNode()
{
	setTitle((char*)u8"파티클 생존시간");
	setStyle(ImFlow::NodeStyle::cyan());

	addOUT<ShaderPin<float>>("")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = "input.Tex2.x";
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		})->renderer(UnLabelPinRenderer);
}

RGBtoSRGBNode::RGBtoSRGBNode()
{
	setTitle((char*)u8"RGB to SRGB");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<Vector3>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<Vector3>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<Vector3>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "pow({}, 1.0 / 2.2)" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector3>{ var };
		})->renderer(UnLabelPinRenderer);
}

SRGBtoRGBNode::SRGBtoRGBNode()
{
	setTitle((char*)u8"SRGB to RGB");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<Vector3>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<Vector3>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<Vector3>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "pow({}, 2.2)" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector3>{ var };
		})->renderer(UnLabelPinRenderer);
}

RGBtoHSVNode::RGBtoHSVNode()
{
	setTitle((char*)u8"RGB to HSV");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<Vector3>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<Vector3>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<Vector3>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "RGBtoHSV({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector3>{ var };
		})->renderer(UnLabelPinRenderer);
}

HSVtoRGBNode::HSVtoRGBNode()
{
	setTitle((char*)u8"HSV to RGB");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<Vector3>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<Vector3>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<Vector3>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "HSVtoRGB({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<Vector3>{ var };
		})->renderer(UnLabelPinRenderer);
}

SinNode::SinNode()
{
	setTitle((char*)u8"Sin");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<float>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<float>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "sin({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		})->renderer(UnLabelPinRenderer);
}

CosNode::CosNode()
{
	setTitle((char*)u8"Cos");
	setStyle(ImFlow::NodeStyle::cyan());
	addIN<ShaderPin<void>>("a", {}, SameType())->renderer(UnLabelPinRenderer);
	addOUT<ShaderPin<float>>("")->behaviour(
		[this]()
		{
			auto a = getInVal<ShaderPin<float>>("a");
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "cos({})" }, a.value->identifier);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			return ShaderPin<float>{ var };
		})->renderer(UnLabelPinRenderer);
}
