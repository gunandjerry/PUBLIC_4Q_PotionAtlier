#pragma once
#include <string>
#include <istream>
#include <ostream>
#include <memory>
#include <filesystem>
#include <Scene/Base/Scene.h>
#include <Manager/ResourceManager.h>
#include <Utility/WinUtility.h>
#include <Utility/SerializedUtility.h>

template<typename AssetData>
class AssetBase abstract
{
public:
	/*Extension*/
	AssetBase(const wchar_t* AssetExtension)
		: extension(AssetExtension) 
	{
#ifndef _DEBUG
		size_t assetHash = typeid(AssetData).hash_code();
		sharedAssetPtr = GetResourceManager<AssetData>().GetResource(std::to_wstring(assetHash).c_str());
#endif // _DEBUG
	}
	virtual ~AssetBase() = default;
private:
	const std::wstring extension; //확장자
	std::wstring assetPath;
	std::shared_ptr<AssetData> sharedAssetPtr;

public:
	std::wstring GetExtension() { return extension; }

	void SaveAsset();
	void SaveAsAsset(const wchar_t* path, bool Override = false);
	void SaveAsAssetWithDialog();

	void OpenAssetWithDialog();
	void OpenAsset(const wchar_t* path);
	const std::wstring& GetAssetPath() { return assetPath; }
	AssetData* GetAssetData() const 
	{ 
		assert(sharedAssetPtr != nullptr && "sharedAssetPtr is nullptr");
		return sharedAssetPtr.get(); 
	}

protected:
	virtual void Serialized(std::ofstream& ofs) = 0;
	virtual void Deserialized(std::ifstream& ifs) = 0;
};

template<typename AssetData>
void AssetBase<AssetData>::SaveAsset()
{
	if (assetPath == L"")
	{
		SaveAsAssetWithDialog();
	}
	else
	{
		std::ofstream ofs(assetPath.c_str(), std::ios::binary | std::ios::trunc);
		if (ofs.is_open())
			Serialized(ofs);

		ofs.close();
	}
}

template<typename AssetData>
void AssetBase<AssetData>::SaveAsAsset(const wchar_t* _path, bool Override)
{
	std::wstring savePath = _path;
	if (savePath != L"")
	{
		std::filesystem::path path(savePath);
		std::filesystem::path fileExtension = path.extension();
		std::wstring dotExtension = L"." + extension;
		if (fileExtension != dotExtension)
		{
			if (fileExtension != L"")
			{
				path.replace_extension(dotExtension);
			}
			else
			{
				path += dotExtension;
			}
		}

		if (!std::filesystem::exists(path))
		{
			if (!std::filesystem::exists(path.parent_path()))
			{
				std::filesystem::create_directories(path.parent_path());
			}
		}
		else if (!Override)
		{
			int result = MessageBox(
				NULL,
				L"파일이 존재합니다. 덮어쓰시겠습니까?",
				path.c_str(),
				MB_OKCANCEL | MB_ICONQUESTION
			);
			if (result == IDCANCEL) {
				return;
			}
		}

		assetPath = path.c_str();
		SaveAsset();
	}
}

template<typename AssetData>
void AssetBase<AssetData>::SaveAsAssetWithDialog()
{
	std::wstring savePath = WinUtility::GetSaveAsFilePath(extension.c_str());
	if (savePath != L"")
	{
		if (std::filesystem::path(savePath).is_absolute())
		{
			savePath = std::filesystem::relative(savePath, std::filesystem::current_path());
		}
		SaveAsAsset(savePath.c_str());
	}
}

template<typename AssetData>
void AssetBase<AssetData>::OpenAssetWithDialog()
{
	std::wstring openPath = WinUtility::GetOpenFilePath(extension.c_str());
	if (openPath != L"")
	{
		if (std::filesystem::path(openPath).is_absolute())
		{
			openPath = std::filesystem::relative(openPath, std::filesystem::current_path());
		}
		OpenAsset(openPath.c_str());
	}
}

template<typename AssetData>
void AssetBase<AssetData>::OpenAsset(const wchar_t* path)
{
	sharedAssetPtr = GetResourceManager<AssetData>().GetResource(path);
	long refCount = sharedAssetPtr.use_count();
	bool startDeserialized = false;
#ifdef _EDITOR
	if (Scene::EditorSetting.IsPlay() && refCount == 1)
		startDeserialized = true;
	else
		startDeserialized = true;
#else
	startDeserialized = true;
#endif // _EDITOR
	assetPath = path;
	if(startDeserialized)
	{
		std::ifstream ifs(path, std::ios::binary);
		if (ifs.is_open())
		{
			Deserialized(ifs);
		}
		ifs.close();
	}
}

