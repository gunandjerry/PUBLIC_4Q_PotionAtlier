#include "SkyBoxRender.h"
#include <Manager/HLSLManager.h>
#include <Component/Render/MeshRender.h>
#include <Utility/ImguiHelper.h>
#include <Utility/WinUtility.h>
#include <Utility/SerializedUtility.h>
#include <Utility/SQLiteLogger.h>

static void ClearRendererBRDF()
{
    if (DefferdRenderer* renderer = &D3D11_GameApp::GetRenderer())
    {
        renderer->RemoveBinadble("BRDF_LUT_PS");
        renderer->RemoveBinadble("BRDF_LUT_CS");
        renderer->RemoveBinadble("Diffuse_IBL_PS");
        renderer->RemoveBinadble("Diffuse_IBL_CS");
        renderer->RemoveBinadble("Specular_IBL_PS");
        renderer->RemoveBinadble("Specular_IBL_CS");
    }
}

static void SetRendererBRDF(
    const Texture* brdf_lut,
    const Texture* diffuse_ibl,
    const Texture* specular_ibl)
{
    if (DefferdRenderer* renderer = &D3D11_GameApp::GetRenderer())
    {
        renderer->AddBinadble("BRDF_LUT_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 30, (ID3D11ShaderResourceView*)(*brdf_lut) });
        renderer->AddBinadble("BRDF_LUT_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 30, (ID3D11ShaderResourceView*)(*brdf_lut) });
        renderer->AddBinadble("Diffuse_IBL_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 31, (ID3D11ShaderResourceView*)(*diffuse_ibl) });
        renderer->AddBinadble("Diffuse_IBL_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 31, (ID3D11ShaderResourceView*)(*diffuse_ibl) });
        renderer->AddBinadble("Specular_IBL_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 32, (ID3D11ShaderResourceView*)(*specular_ibl) });
        renderer->AddBinadble("Specular_IBL_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 32, (ID3D11ShaderResourceView*)(*specular_ibl) });
    }
}

SkyBoxRender* SkyBoxRender::GetMainSkyBox()
{
	if (mainSkyBox && mainSkyBox->Enable && mainSkyBox->gameObject.Active)
	{
        return mainSkyBox->materialAsset.GetTexturesV2().empty() ? nullptr : mainSkyBox;
	}
    return nullptr;
}

SkyBoxRender::~SkyBoxRender()
{
    if (mainSkyBox == this)
    {
        mainSkyBox = nullptr;
        ClearRendererBRDF();
    }
}

void SkyBoxRender::Awake()
{
    if (mainSkyBox)
    {
        Debug_printf("Warning : There can only be one skybox.\n"); //스카이 박스는 하나만 존재 가능합니다.
        SQLiteLogger::EditorLog("warning", "There can only be one skybox.");
        GameObject::Destroy(this->gameObject);
        return;
    }

    // Front face
    vertices.emplace_back(-1.0f, -1.0f, -1.0f, 1.0f);
    vertices.emplace_back(1.0f, -1.0f, -1.0f, 1.0f);
    vertices.emplace_back(1.0f, 1.0f, -1.0f, 1.0f);
    vertices.emplace_back(-1.0f, 1.0f, -1.0f, 1.0f);

    // Back face
    vertices.emplace_back(-1.0f, -1.0f, 1.0f, 1.0f);
    vertices.emplace_back(1.0f, -1.0f, 1.0f, 1.0f);
    vertices.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
    vertices.emplace_back(-1.0f, 1.0f, 1.0f, 1.0f);

    // Indices for the cube (two triangles per face)
    // Front face
    indices.emplace_back(0); indices.emplace_back(1); indices.emplace_back(2);
    indices.emplace_back(0); indices.emplace_back(2); indices.emplace_back(3);

    // Back face
    indices.emplace_back(4); indices.emplace_back(6); indices.emplace_back(5);
    indices.emplace_back(4); indices.emplace_back(7); indices.emplace_back(6);

    // Left face
    indices.emplace_back(4); indices.emplace_back(5); indices.emplace_back(1);
    indices.emplace_back(4); indices.emplace_back(1); indices.emplace_back(0);

    // Right face
    indices.emplace_back(3); indices.emplace_back(2); indices.emplace_back(6);
    indices.emplace_back(3); indices.emplace_back(6); indices.emplace_back(7);

    // Top face
    indices.emplace_back(1); indices.emplace_back(5); indices.emplace_back(6);
    indices.emplace_back(1); indices.emplace_back(6); indices.emplace_back(2);

    // Bottom face
    indices.emplace_back(4); indices.emplace_back(0); indices.emplace_back(3);
    indices.emplace_back(4); indices.emplace_back(3); indices.emplace_back(7);

    CreateMesh();

    //SkyBox용 기본
    materialAsset.OpenAsset(TempResourcePath);
    materialAsset.clearTextures();
    materialAsset.clearSamplers();

    //SkyBox용 샘플러
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // 선형 필터링 (Mipmap 사용 시 선형 보간)
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;    // 좌측, 우측 경계에서 클램프
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;    // 위쪽, 아래쪽 경계에서 클램프
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;    // 큐브 맵의 W축도 클램프
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;   // 비교 함수는 필요하지 않음 (단순 샘플링)
    samplerDesc.BorderColor[0] = 1.0f;                     // 경계 색상 (클램프가 동작할 때 사용)
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MipLODBias = 0.0f;                         // Mipmap LOD 바이어스
    samplerDesc.MaxAnisotropy = 1;                         // 비선형 샘플링을 사용하지 않음
    samplerDesc.MinLOD = 0.0f;                             // 최소 LOD는 0
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // Mipmap의 최대 LOD
    materialAsset.SetSamplerState(samplerDesc, 0);

    {
        using namespace std::string_literals;
        std::wstring vertexPath(HLSLManager::EngineShaderPath + L"SkyBoxVS.hlsl"s);
        SetVS(vertexPath.c_str());

        std::wstring pixelPath(HLSLManager::EngineShaderPath + L"SkyBoxPS.hlsl"s);
        SetPS(pixelPath.c_str());
    }
    mainSkyBox = this;
}

void SkyBoxRender::FixedUpdate()
{
}

void SkyBoxRender::Update()
{

}

void SkyBoxRender::LateUpdate()
{
}

void SkyBoxRender::Render()
{
   //TransformBufferData 업데이트(더티 체크 필요할듯?)
    transformBuffer.Set(
        TransformBufferData
        {
            .World = XMMatrixTranspose(transform.GetWM()),
            .WorldInverseTranspose = transform.GetIWM()
        }
    );

    //등록
    meshDrawCommand.meshData.shaderResources.push_back(
        Binadble
        {
            .shaderType = EShaderType::Vertex,
            .bindableType = EShaderBindable::ConstantBuffer,
            .slot = 0,
            .bind = (ID3D11Buffer*)transformBuffer
        }
    );

    //텍스쳐 등록
    size_t textureCount = materialAsset.GetTexturesV2().size();
    const auto& textures = materialAsset.GetTexturesV2();
    const auto& textureSlot = materialAsset.GetTexturesSlot();
    for (size_t i = 0; i < textureCount; i++)
    {


        Binadble bind{};
        bind.bindableType = EShaderBindable::ShaderResource;
        bind.shaderType = EShaderType::Pixel;
        bind.slot = textureSlot[i];
        bind.bind = (ID3D11ShaderResourceView*)textures[i];
        meshDrawCommand.materialData.shaderResources.push_back(bind);
    }

    //샘플러 등록
    size_t samplersCount = materialAsset.GetSamplers().size();
    const auto& samplers = materialAsset.GetSamplers();
    const auto& samplerSlot = materialAsset.GetSamplerSlot();
    for (size_t i = 0; i < samplersCount; i++)
    {
        Binadble bind{};
        bind.bindableType = EShaderBindable::Sampler;
        bind.shaderType = EShaderType::Pixel;
        bind.slot = samplerSlot[i];
        bind.bind = (ID3D11SamplerState*)samplers[i];
        meshDrawCommand.materialData.shaderResources.push_back(bind);
    }

    D3D11_GameApp::GetRenderer().SetSkyBoxDrawCommand(meshDrawCommand);
}

void SkyBoxRender::SetSkyBox(TEXTURE_TYPE type, const wchar_t* path)
{
    std::filesystem::path loadPath = path;
	if (loadPath.is_absolute())
	{
        loadPath = std::filesystem::relative(loadPath, std::filesystem::current_path());
		path = loadPath.c_str();
	}
    if (!std::filesystem::exists(path)) return;;
    
    if(TEXTURE_TYPE::ENV == type)
        materialAsset.SetCubeMapTexture(path, type);
    else
        materialAsset.SetTexture2D(path, type);

    const Texture* brdf_lut = nullptr;
    const Texture* diffuse_ibl = nullptr;
    const Texture* specular_ibl = nullptr;
    const auto& textuers = materialAsset.GetTexturesV2();
    const auto& textuerSlot = materialAsset.GetTexturesSlot();
    size_t textuerCount = textuers.size();
    for (size_t i = 0; i < textuerCount; i++)
    {
        TEXTURE_TYPE type = (TEXTURE_TYPE)textuerSlot[i];
        switch (type)
        {
        case SkyBoxRender::BRDF_LUT:
            brdf_lut = &textuers[i];
            break;
        case SkyBoxRender::Diffuse_IBL:
            diffuse_ibl = &textuers[i];
            break;
        case SkyBoxRender::Specular_IBL:
            specular_ibl = &textuers[i];
            break;
        default:
            continue;
        }
    }

    ClearRendererBRDF();
    if(brdf_lut && diffuse_ibl && specular_ibl)
    {
        SetRendererBRDF(brdf_lut, diffuse_ibl, specular_ibl);
    }
}

void SkyBoxRender::ResetSkyBox(TEXTURE_TYPE type)
{
    materialAsset.ReleaseTexture(type);

    const Texture* brdf_lut = nullptr;
    const Texture* diffuse_ibl = nullptr;
    const Texture* specular_ibl = nullptr;
    const auto& textuers = materialAsset.GetTexturesV2();
    const auto& textuerSlot = materialAsset.GetTexturesSlot();
    size_t textuerCount = textuers.size();
    for (size_t i = 0; i < textuerCount; i++)
    {
        TEXTURE_TYPE type = (TEXTURE_TYPE)textuerSlot[i];
        switch (type)
        {
        case SkyBoxRender::BRDF_LUT:
            brdf_lut = &textuers[i];
            break;
        case SkyBoxRender::Diffuse_IBL:
            diffuse_ibl = &textuers[i];
            break;
        case SkyBoxRender::Specular_IBL:
            specular_ibl = &textuers[i];
            break;
        default:
            continue;
        }
    }

    ClearRendererBRDF();
    if (brdf_lut && diffuse_ibl && specular_ibl)
    {
        SetRendererBRDF(brdf_lut, diffuse_ibl, specular_ibl);
    }
}

void SkyBoxRender::SetVS(const wchar_t* path)
{
    currVSpath = path;
    {
        ComPtr<ID3D11VertexShader> vs;
        ComPtr<ID3D11InputLayout> il;
        hlslManager.CreateSharingShader(currVSpath.c_str(), &vs, &il);
        meshDrawCommand.meshData.vertexShader.LoadShader(vs.Get(), il.Get());
    }
}

void SkyBoxRender::SetPS(const wchar_t* path)
{
    currPSpath = path;
    {
        ComPtr<ID3D11PixelShader> ps;
        hlslManager.CreateSharingShader(currPSpath.c_str(), &ps);
        meshDrawCommand.materialData.pixelShader.LoadShader(ps.Get());
    }
}

void SkyBoxRender::CreateMesh()
{
    using namespace Utility;
    if (vertices.empty() || indices.empty())
        return;

    ID3D11Device* pDevcie = RendererUtility::GetDevice();

    meshDrawCommand.meshData.vertexStride = sizeof(Vector4);
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = sizeof(Vector4) * vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();
    ComPtr<ID3D11Buffer> vb;
    Check(pDevcie->CreateBuffer(&bd, &vbData, &vb));
    meshDrawCommand.meshData.vertexBuffer.Load(vb);

    meshDrawCommand.meshData.indexCounts = indices.size();
    bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(UINT) * indices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();
    ComPtr<ID3D11Buffer> ib;
    CheckHRESULT(pDevcie->CreateBuffer(&bd, &ibData, &ib));
    meshDrawCommand.meshData.indexBuffer.Load(ib);

    vertices.clear();
    indices.clear();
}

void SkyBoxRender::Serialized(std::ofstream& ofs)
{
    using namespace Binary;
    std::wstring env;
    std::wstring brdf_lut;
    std::wstring diffuse_ibl;
    std::wstring specular_ibl;
    const auto& textuersPath = materialAsset.GetCurrTexturesPath();
    const auto& textuerSlot = materialAsset.GetTexturesSlot();
    size_t textuerCount = textuersPath.size();
    for (size_t i = 0; i < textuerCount; i++)
    {
        TEXTURE_TYPE type = (TEXTURE_TYPE)textuerSlot[i];
        switch (type)
        {
        case TEXTURE_TYPE::ENV:
            env = textuersPath[i];
            break;
        case SkyBoxRender::BRDF_LUT:
            brdf_lut = textuersPath[i];
            break;
        case SkyBoxRender::Diffuse_IBL:
            diffuse_ibl = textuersPath[i];
            break;
        case SkyBoxRender::Specular_IBL:
            specular_ibl = textuersPath[i];
            break;
        }
    }
    Write::wstring(ofs, env);
    Write::wstring(ofs, brdf_lut);
    Write::wstring(ofs, diffuse_ibl);
    Write::wstring(ofs, specular_ibl);
}

void SkyBoxRender::Deserialized(std::ifstream& ifs)
{
    using namespace Binary;
    std::wstring path = Read::wstring(ifs);
    path.empty() ? void() : SetSkyBox(TEXTURE_TYPE::ENV, path.c_str());
    path = Read::wstring(ifs);
    path.empty() ? void() : SetSkyBox(TEXTURE_TYPE::BRDF_LUT, path.c_str());
    path = Read::wstring(ifs);
    path.empty() ? void() : SetSkyBox(TEXTURE_TYPE::Diffuse_IBL, path.c_str());
    path = Read::wstring(ifs);
    path.empty() ? void() : SetSkyBox(TEXTURE_TYPE::Specular_IBL, path.c_str());
}

void SkyBoxRender::InspectorImguiDraw()
{
    auto setSkyBox = [this](TEXTURE_TYPE type)
        {
            std::wstring loadPath = WinUtility::GetOpenFilePath(L"dds");
            SetSkyBox(type, loadPath.c_str());
        };
    if (ImGui::Button("Set EVN"))
    {
        setSkyBox(TEXTURE_TYPE::ENV);
    }
    ImGui::SameLine();
    if (ImGui::Button("Set BRDF_LUT"))
    {
        setSkyBox(TEXTURE_TYPE::BRDF_LUT);
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Diffuse_IBL"))
    {
        setSkyBox(TEXTURE_TYPE::Diffuse_IBL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Specular_IBL"))
    {
        setSkyBox(TEXTURE_TYPE::Specular_IBL);
    }
    if (ImGui::Button("Clear SkyBox"))
    {
        ClearRendererBRDF();
    }
}