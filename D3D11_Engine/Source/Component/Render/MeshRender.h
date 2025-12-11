#pragma once
#include <Component/Base/RenderComponent.h>
#include <string>
#include <Asset/MaterialAsset.h>

struct TransformBufferData
{
	alignas(16) Matrix World;
	alignas(16) Matrix WorldInverseTranspose;
};

class ShaderNodeEditor;
class MeshRender : public RenderComponent
{
	inline static std::list<MeshRender*> instanceList;
	std::list<MeshRender*>::iterator myIter;
public:
	static void ReloadShaderAll();
	static void ExportMaterialAll();
public:
	MeshRender();
	virtual ~MeshRender() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
public:
	virtual void Awake() = 0;
	virtual void InspectorImguiDraw();

	std::pair<bool, bool> ExistsDefaultMaterialNodeAndAsset(const wchar_t* materialPath) const;
	void CreateDefaultMaterial(const wchar_t* materialPath);
	std::unique_ptr<ShaderNodeEditor> GetDefaultNodeEditor() const;
	std::filesystem::path MakeDefaultMaterialNodePath(const wchar_t* materialPath) const;
	std::filesystem::path MakeDefaultMaterialAssetPath(const wchar_t* materialPath) const;

	void ReloadShader();
	void ExportMaterialNode();
protected:
	virtual void DefaultMaterialEvent(std::unique_ptr<ShaderNodeEditor>& editor) {}

protected:
	MeshDrawCommand				meshDrawCommand;
	ConstantBuffer				transformBuffer;
	ConstantBuffer				material;

public:
	MaterialAsset materialAsset;

protected:
	virtual void FixedUpdate() = 0;
	virtual void Update() = 0;
	virtual void LateUpdate() = 0;
	virtual void Render() final;
public:
	virtual void CreateMesh() = 0;
	virtual void UpdateMeshDrawCommand() = 0;

public:
	void SetVS(const wchar_t* path);
	inline const std::wstring& GetVSpath() { return currVSpath; }

	void SetPS(const wchar_t* path);
	inline const std::wstring& GetPSpath() { return currPSpath; }

private:
	std::wstring currVSpath;
	std::wstring currPSpath;

public:
	void SetMeshID(const std::string& meshID);
	inline const std::string& GetMeshID() const { return MeshID; }

protected:
	/*Serialized 이후 호출되는 가상함수*/
	virtual void SaveMeshResource(std::filesystem::path& meshResourcePath) = 0;
	/*SetMeshID 이후 호출되는 가상함수*/
	virtual void LoadMeshResource(std::filesystem::path& meshResourcePath) = 0;

	std::filesystem::path GetMeshResourcePath() const;

	/*리소스 관리용*/
	struct MeshBufferResource
	{
		uint32_t indexCounts;
		uint32_t vertexStride;
		ComPtr<ID3D11Buffer> indexBufferResource;
		ComPtr<ID3D11Buffer> vertexBufferResource;
	};
	std::shared_ptr<MeshBufferResource> meshBufferResource;
private:
	std::string MeshID;
	std::wstring defaultMaterialPath;

public:
	/*메시 머터리얼의 BaseColor 기억용. CopyFBX에서 해당 컬러를 복사해갑니다.*/
	DirectX::SimpleMath::Color baseColor{1,1,1,1};
};