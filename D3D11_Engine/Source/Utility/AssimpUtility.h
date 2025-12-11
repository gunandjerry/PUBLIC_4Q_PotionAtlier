#pragma once
#include <memory>
#include <Math\AssimpMath.h>
#include <Core/Transform.h>
#include <functional>
#include <filesystem>
#include <map>

class MeshRender;
class GameObject;

enum class SURFACE_TYPE
{
	PBR,
	NONE
};

namespace Utility
{
	bool ParseFileName(aiString& str);

	std::map<std::wstring, std::vector<MeshRender*>> CollectMeshComponents(GameObject* root);

	void LoadFBX(const wchar_t* path, 
		bool isStatic, 
		SURFACE_TYPE surface,
		GameObject** pRootObjectOut = nullptr
		);

	void LoadFBXResource(const wchar_t* path,
		bool isStatic, 
		SURFACE_TYPE surface);

	std::filesystem::path GetTempResourcePath(const wchar_t* flieName, bool isStatic);
}
