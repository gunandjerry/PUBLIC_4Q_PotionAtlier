#pragma once
#include <Asset/Base/AssetBase.h>
#include <DrawCommand.h>
#include <map>
#include <d3dcompiler.h>

struct ID3D11ShaderReflection;

enum class ETextureType 
{
	Albedo,
	Normal,
	Specular,
	Emissive,
	Opacity,
	Metalness,
	Roughness,
	AmbientOcculusion,
	Null
};

struct CustomData 
{
	struct FieldData
	{
		size_t offset;
		size_t size;
	};
	void SetSize(size_t size)
	{
		data.resize(size);
	}

	void PushField(std::string filedName, size_t offset, size_t size)
	{
		fieldData[filedName].offset = offset;
		fieldData[filedName].size = size;

		if (data.size() < offset + size)
		{
			//32바이트 단위 리사이즈

			data.resize((offset + size + 15) & ~15);
		}
	}


	template <typename T>
	T* GetField(std::string filedName)
	{
		auto iter = fieldData.find(filedName);
		if (iter != fieldData.end())
		{
			return (T*)&data[iter->second.offset];
		}
		return nullptr;
	}

	template <typename T>
	void SetField(std::string filedName, T value)
	{
		auto iter = fieldData.find(filedName);
		if (iter != fieldData.end())
		{
			*(T*)&data[iter->second.offset] = value;
		}

	}

	void* Data()
	{
		return data.data();
	}

	size_t GetSize()
	{
		return data.size();
	}

	std::map<std::string, FieldData>& GetFieldData()
	{
		return fieldData;
	}

private:
	std::map<std::string, FieldData> fieldData;
	std::vector<char> data;
};

struct MaterialAssetData
{
	friend class MaterialAsset;

	MaterialAssetData();
	~MaterialAssetData();


private:
	std::string					pixelShaderData;
	PixelShader					pixelShader;
	std::vector<uint32_t>		texturesSlot;
	std::vector<Texture>		texturesV2;
	std::vector<std::wstring>	currTexturePath;

	std::vector<uint32_t>		samplerSlot;
	std::vector<SamplerState>	samplers;
	bool isForward;
};

class MaterialAsset : public AssetBase<MaterialAssetData>
{
public:
	inline static constexpr const wchar_t* AssetExtension = L"MaterialAsset";
	inline static constexpr const wchar_t* DotAssetExtension =  L".MaterialAsset";
	MaterialAsset();
	virtual ~MaterialAsset() override;

public:
	void CopyAsset(const MaterialAsset& rhs);
	
	void ReleaseSamplerState(uint32_t slot);
	void SetSamplerState(const D3D11_SAMPLER_DESC& desc, uint32_t slot);
	void clearSamplers();
public:
	bool SetPixelShader(const std::string& shaderCode, bool isForward);
	bool LoadPixelShader(const std::string& shaderCode, ComPtr<ID3D11PixelShader>& rhiPixelShader, ComPtr<ID3D11ShaderReflection>& reflector);
	void ClearCache();

	template <typename T>
	void SetCubeMapTexture(const wchar_t* path, T type)
	{
		uint32_t slot = static_cast<uint32_t>(type);
		SetCubeMapTexture(path, slot);
	}
	void SetCubeMapTexture(const wchar_t* path, uint32_t slot);

	void SetTexture2D(const wchar_t* path, uint32_t slot);
	template <typename T> void SetTexture2D(const wchar_t* path, T type)	
	{
		uint32_t slot = static_cast<uint32_t>(type);
		SetTexture2D(path, slot);
	}

	void ReleaseTexture(uint32_t slot);
	template <typename T> void ReleaseTexture(T type)
	{
		uint32_t slot = static_cast<uint32_t>(type);
		ReleaseTexture(slot);
	}

	void clearTextures();
	CustomData customData;
protected:
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;

private:
	RendererBuffer customBuffer;

public:
	const PixelShader& GetPS() { return GetAssetData()->pixelShader; }
	const std::vector<uint32_t>& GetTexturesSlot() { return GetAssetData()->texturesSlot; }
	const std::vector<Texture>& GetTexturesV2() { return GetAssetData()->texturesV2; }
	const std::vector<std::wstring>& GetCurrTexturesPath() { return GetAssetData()->currTexturePath; }

	const std::vector<uint32_t>& GetSamplerSlot() { return GetAssetData()->samplerSlot; }
	const std::vector<SamplerState>& GetSamplers() { return GetAssetData()->samplers; }
	RendererBuffer& GetCustomBuffer() { return customBuffer; }
};