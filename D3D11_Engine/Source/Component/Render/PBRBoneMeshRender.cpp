#include "PBRBoneMeshRender.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>

PBRBoneMeshRender::PBRBoneMeshRender()
{

}

void PBRBoneMeshRender::Awake()
{
    meshObject = dynamic_cast<PBRMeshObject*>(&gameObject);
    if (!meshObject)
    {
        Debug_wprintf(L"Warning : PBRBoneMeshRender can only be added to PBRMeshObject.\n");
        GameObject::DestroyComponent(this);
        return;
    }

	SimpleBoneMeshRender::Awake();
	{
		using namespace std::string_literals;
		std::wstring vertexPath(HLSLManager::EngineShaderPath + L"VertexSkinningShader.hlsl"s);
		SetVS(vertexPath.c_str());

		std::wstring pixelPath(HLSLManager::EngineShaderPath + L"PBROpaquePS.hlsl"s);
		SetPS(pixelPath.c_str());
	}
}

void PBRBoneMeshRender::UpdateMeshDrawCommand()
{
    SimpleBoneMeshRender::UpdateMeshDrawCommand();

}
