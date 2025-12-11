#include "SphereObject.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>
#include <Utility/AssimpUtility.h>
#include <Component/Render/PBRMeshRender.h>
#include <filesystem>

void SphereObject::Awake()
{
	PBRMeshObject::Awake();
	{
		using namespace std::string_literals;
		static bool isInit = false;

		if (isInit)
		{
			std::filesystem::path objectPath(Utility::GetTempResourcePath(L"sphere.fbx", false));
			GameObject* Sphere = gameObjectFactory.DeserializedObject(objectPath.c_str());
			GameObject& SphereChild = Sphere->transform.GetChild(0)->gameObject;
			SphereChild.transform.SetParent(nullptr);
			SphereChild.transform.scale = 0.05f;
			Destroy(Sphere);
		}		
		else
		{
			std::wstring fbxPath(HLSLManager::EngineShaderPath + L"sphere.fbx"s);
			GameObject* Sphere = nullptr;
			Utility::LoadFBX(fbxPath.c_str(), false, SURFACE_TYPE::PBR, &Sphere);
			GameObject& SphereChild = Sphere->transform.GetChild(0)->gameObject;
			SphereChild.transform.SetParent(nullptr);
			SphereChild.transform.scale = 0.05f;
			Destroy(Sphere);
			isInit = true;
		}
		Destroy(this);
	}
}
