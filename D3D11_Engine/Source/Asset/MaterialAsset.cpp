#include "MaterialAsset.h"
#include <framework.h>
#include <Utility/SerializedUtility.h>
#include <d3dcompiler.h>

MaterialAsset::MaterialAsset()
	: AssetBase(AssetExtension)
{

}

MaterialAsset::~MaterialAsset() = default;

void MaterialAsset::CopyAsset(const MaterialAsset& rhs)
{
	if (this == &rhs)
		return;

	MaterialAssetData* data = GetAssetData();
	MaterialAssetData* rhsData = rhs.GetAssetData();

	data->texturesSlot = rhsData->texturesSlot;
	data->texturesV2 = rhsData->texturesV2;
	data->currTexturePath = rhsData->currTexturePath;
	for (auto& path : rhsData->currTexturePath)
	{
		ID3D11ShaderResourceView* temp;
		textureManager.CreateSharingTexture(path.c_str(), &temp); //refcounter 증가용
	}

	data->samplerSlot = rhsData->samplerSlot;
	data->samplers = rhsData->samplers;

	data->pixelShaderData = rhsData->pixelShaderData;
	data->pixelShader = rhsData->pixelShader;
}

void MaterialAsset::SetSamplerState(const D3D11_SAMPLER_DESC& desc, uint32_t slot)
{
	MaterialAssetData* data = GetAssetData();

	ReleaseSamplerState(slot);
	data->samplerSlot.emplace_back(slot);
	data->samplers.emplace_back();
	SamplerState& sampler = data->samplers.back();
	sampler.SetSampler(desc);
}

void MaterialAsset::clearSamplers()
{
	MaterialAssetData* data = GetAssetData();
	data->samplers.clear();
	data->samplerSlot.clear();
}

void MaterialAsset::ReleaseSamplerState(uint32_t slot)
{
	MaterialAssetData* data = GetAssetData();

	for (size_t i = 0; i < data->samplerSlot.size(); i++)
	{
		if (data->samplerSlot[i] == slot)
		{
			data->samplers.erase(data->samplers.begin() + i);
			data->samplerSlot.erase(data->samplerSlot.begin() + i);
			break;
		}
	}
}

bool MaterialAsset::SetPixelShader(const std::string& shaderCode, bool isForward)
{
	MaterialAssetData* data = GetAssetData();

	data->isForward = isForward;
	data->pixelShaderData = shaderCode;

	ComPtr<ID3D11PixelShader> rhiPixelShader;
	ComPtr<ID3D11ShaderReflection> reflector;
	if (shaderCode.empty()) return false;

	if (!LoadPixelShader(shaderCode, rhiPixelShader, reflector))
	{
		hlslManager.CreateShader(shaderCode.data(), shaderCode.size(), &rhiPixelShader, &reflector);
	}

	data->pixelShader.LoadShader(rhiPixelShader.Get());
	data->pixelShader.isForward = isForward;

	if (!rhiPixelShader)
	{
		MessageBox(nullptr, L"PixelShader Load Fail", L"Error", MB_OK); 
		return false;
	}
	if (!reflector) return false;

	D3D11_SHADER_DESC shaderInfo;
	reflector->GetDesc(&shaderInfo);

	for (size_t i = 0; i < shaderInfo.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
		reflector->GetResourceBindingDesc(i, &resourceDesc);

		if (resourceDesc.Type != D3D_SIT_CBUFFER) continue;

		ID3D11ShaderReflectionConstantBuffer* constantBuffer = reflector->GetConstantBufferByName(resourceDesc.Name);
		D3D11_SHADER_BUFFER_DESC bufferDesc = {};
		constantBuffer->GetDesc(&bufferDesc);

		if (!bufferDesc.Name || bufferDesc.Name != std::string("CustomBuffer")) continue;

		customData.GetFieldData().clear();
		customData.SetSize(bufferDesc.Size);
		customBuffer.Init(D3D11_BUFFER_DESC{ .Usage = D3D11_USAGE_DEFAULT, .BindFlags = D3D11_BIND_CONSTANT_BUFFER },
						  customData.GetSize(),
						  customData.Data());

		for (UINT j = 0; j < bufferDesc.Variables; j++)
		{
			ID3D11ShaderReflectionVariable* shaderValue = constantBuffer->GetVariableByIndex(j);
			auto shaderTypeRefelct = shaderValue->GetType();
			D3D11_SHADER_TYPE_DESC typeDesc;
			shaderTypeRefelct->GetDesc(&typeDesc);
			for (UINT k = 0; k < typeDesc.Members; k++)
			{
				D3D11_SHADER_TYPE_DESC memberTypeDesc;
				D3D11_SHADER_VARIABLE_DESC memberValueDesc;
				shaderTypeRefelct->GetMemberTypeByIndex(k)->GetDesc(&memberTypeDesc);
				
				customData.PushField(shaderTypeRefelct->GetMemberTypeName(k), memberTypeDesc.Offset, memberTypeDesc.Rows * memberTypeDesc.Columns * 4);
			}

		}
	}
	return true;
}

bool MaterialAsset::LoadPixelShader(const std::string& shaderCode, ComPtr<ID3D11PixelShader>& rhiPixelShader, ComPtr<ID3D11ShaderReflection>& reflector)
{
	using namespace std;
	namespace fs = std::filesystem;

	ComPtr< ID3DBlob> blob;

	fs::path relativePath = fs::path(GetAssetPath());

	// 상위 디렉토리 ".." 제거
	fs::path sanitizedPath;
	for (const auto& part : relativePath)
	{
		if (part == "..") continue;        // 상위 경로 제거
		if (part == ".") continue;         // 현재 디렉토리 제거
		sanitizedPath /= part;
	}

	fs::path cachePath = fs::path("CacheShader") / sanitizedPath;
	cachePath.replace_extension(".cso");

	std::error_code ec;
	fs::create_directories(cachePath.parent_path(), ec);
	if (ec) return false;

	if (fs::exists(cachePath))
	{
		// 캐시된 블룹 파일 로드
		if (FAILED(D3DReadFileToBlob(cachePath.wstring().c_str(), &blob)))
			return false;
	}
	else
	{
		auto result = Utility::CompileShader(hlslManager.GetIncludePath(), shaderCode.data(), shaderCode.size(), "main", "ps_5_0", &blob);
		if (FAILED(result)) return false;

		// 블룹저장
		if (FAILED(D3DWriteBlobToFile(blob.Get(), cachePath.wstring().c_str(), true)))
			return false;

	}
	(RendererUtility::GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &rhiPixelShader));
	(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&reflector)));

	return true;
}

void MaterialAsset::ClearCache()
{
	using namespace std;
	namespace fs = std::filesystem;

	fs::path relativePath = fs::path(GetAssetPath());

	// 상위 디렉토리 ".." 제거
	fs::path sanitizedPath;
	for (const auto& part : relativePath)
	{
		if (part == "..") continue;        // 상위 경로 제거
		if (part == ".") continue;         // 현재 디렉토리 제거
		sanitizedPath /= part;
	}

	fs::path cachePath = fs::path("CacheShader") / sanitizedPath;
	cachePath.replace_extension(".cso");

	if (fs::exists(cachePath))
	{
		fs::remove(cachePath);
	}
}


void MaterialAsset::SetCubeMapTexture(const wchar_t* path, uint32_t slot)
{
	MaterialAssetData* data = GetAssetData();

	ReleaseTexture(slot);
	data->currTexturePath.emplace_back(path);
	data->texturesSlot.emplace_back(slot);
	data->texturesV2.emplace_back();

	Texture& textures = data->texturesV2.back();
	ComPtr<ID3D11ShaderResourceView> textuer2D;
	textureManager.CreateSharingCubeMap(path, &textuer2D);
	textures.LoadTexture(textuer2D.Get());
	textuer2D->AddRef();
}

void MaterialAsset::SetTexture2D(const wchar_t* path, uint32_t slot)
{
	MaterialAssetData* data = GetAssetData();

	ReleaseTexture(slot);
	data->currTexturePath.emplace_back(path);
	data->texturesSlot.emplace_back(slot);
	data->texturesV2.emplace_back();

	Texture& textures = data->texturesV2.back();
	ComPtr<ID3D11ShaderResourceView> textuer2D;
	try
	{
		textureManager.CreateSharingTexture(path, &textuer2D);
	}
	catch (const Utility::com_exception& comException)
	{
		data->currTexturePath.pop_back();
		data->texturesSlot.pop_back();
		data->texturesV2.pop_back();
		throw comException;
	}
	textures.LoadTexture(textuer2D.Get());
	textuer2D->AddRef();
}

void MaterialAsset::ReleaseTexture(uint32_t slot)
{
	MaterialAssetData* data = GetAssetData();

	std::wstring* path = nullptr;
	size_t index = 0;
	for (size_t i = 0; i < data->texturesSlot.size(); i++)
	{
		if (data->texturesSlot[i] == slot)
		{
			path = &(data->currTexturePath[i]);
			index = i;
			break;
		}
	}
	if (path)
	{
		data->texturesV2.erase(data->texturesV2.begin() + index);
		textureManager.ReleaseSharingTexture(path->c_str());
		data->currTexturePath.erase(data->currTexturePath.begin() + index);
		data->texturesSlot.erase(data->texturesSlot.begin() + index);
	}
}

void MaterialAsset::clearTextures()
{
	auto* data = GetAssetData();
	for (auto& path : data->currTexturePath)
	{
		textureManager.ReleaseSharingTexture(path.c_str());
	}
	data->texturesV2.clear();
	data->currTexturePath.clear();
	data->texturesSlot.clear();
}

void MaterialAsset::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	MaterialAssetData* data = GetAssetData();

	size_t slotSize = data->texturesSlot.size();
	Write::data(ofs, slotSize);
	Write::std_vector(ofs, data->texturesSlot);

	size_t texturesSize = data->texturesV2.size();
	Write::data(ofs, texturesSize);


	size_t pathSize = data->currTexturePath.size();
	Write::data(ofs, pathSize);
	for (auto& path : data->currTexturePath)
	{
		Write::wstring(ofs, path);
	}
	Write::string(ofs, data->pixelShaderData);
	Write::data(ofs, data->isForward);

	Write::data(ofs,0);
	/*Write::data(ofs, data->customData.GetFieldData().size());
	for (auto& [key, value] : data->customData.GetFieldData())
	{
		Write::string(ofs, key);
		Write::data(ofs, value.offset);
		Write::data(ofs, value.size);
	}*/

	
	
}

void MaterialAsset::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	MaterialAssetData* data = GetAssetData();

	data->texturesV2.clear();
	for (size_t i = 0; i < data->currTexturePath.size(); i++)
	{
		if (data->currTexturePath[i].size())
		{
			textureManager.ReleaseSharingTexture(data->currTexturePath[i].c_str());
		}
	}
	data->currTexturePath.clear();
	data->texturesSlot.clear();
	std::vector<uint32_t>		tempTexturesSlot;
	std::vector<std::wstring>	tempCurrTexturePath;

	tempTexturesSlot.resize(Read::data<size_t>(ifs));
	Read::std_vector(ifs, tempTexturesSlot);
	auto size = Read::data<size_t>(ifs);

	tempCurrTexturePath.resize(Read::data<size_t>(ifs));
	for (size_t i = 0; i < tempCurrTexturePath.size(); i++)
	{
		tempCurrTexturePath[i] = Read::wstring(ifs);
	}
	
	for (size_t i = 0; i < tempCurrTexturePath.size(); i++)
	{
		bool retry = true;	
		while (retry)
		{
			try
			{
				SetTexture2D(tempCurrTexturePath[i].c_str(), tempTexturesSlot[i]);
				retry = false;
			}
			catch (const Utility::com_exception& comException)
			{
				HRESULT hresult = comException.GetHRESULT();
				if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hresult ||
					HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hresult)
				{
					std::wstring extention = std::filesystem::path(tempCurrTexturePath[i]).extension();
					std::wstring fileName = std::filesystem::path(tempCurrTexturePath[i]).filename();
					tempCurrTexturePath[i] = WinUtility::GetOpenFilePath(extention.c_str() + 1, fileName.c_str());
					if (!tempCurrTexturePath[i].empty())
					{
						tempCurrTexturePath[i] = std::filesystem::relative(tempCurrTexturePath[i], std::filesystem::current_path());
						retry = true;
					}
					else
					{
						retry = false;	
					}			
				}
				else
				{
					throw comException;
				}
			}
		}
	}
	data->pixelShaderData = Read::string(ifs );
	data->isForward = Read::data<bool>(ifs);
	SetPixelShader(data->pixelShaderData, data->isForward);

	//
	size_t customDataSize = Read::data<size_t>(ifs);

	for (size_t i = 0; i < customDataSize; i++)
	{
		std::string key = Read::string(ifs);
		size_t dataOffset = Read::data<size_t>(ifs);
		size_t dataSize = Read::data<size_t>(ifs);
		//data->customData.PushField(key, dataOffset, dataSize);
	}



}

MaterialAssetData::MaterialAssetData()
{
	isForward = false;
}
MaterialAssetData::~MaterialAssetData()
{
	texturesV2.clear();
	for (auto& path : currTexturePath)
	{
		textureManager.ReleaseSharingTexture(path.c_str());
	}
}

