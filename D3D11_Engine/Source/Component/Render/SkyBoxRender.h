#pragma once
#include <Component/Base/RenderComponent.h>
#include <Asset/MaterialAsset.h>

class SkyBoxRender : public RenderComponent
{
	friend class SceneManager;
	inline static SkyBoxRender* mainSkyBox = nullptr;
	inline static constexpr const wchar_t TempResourcePath[] = L"Resource/SkyBoxTemp/SkyBox.MaterialAsset";
public:
	static SkyBoxRender* GetMainSkyBox();
	enum TEXTURE_TYPE
	{
		ENV = 0,
		BRDF_LUT = 30,
		Diffuse_IBL = 31,
		Specular_IBL = 32,
	};

public:
	SkyBoxRender() = default;
	virtual ~SkyBoxRender() override;

	virtual void Awake() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
	virtual void InspectorImguiDraw() override;
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Render() override;

public:
	void SetSkyBox(TEXTURE_TYPE type, const wchar_t* path);
	void ResetSkyBox(TEXTURE_TYPE type);

public:
	void SetVS(const wchar_t* path);
	inline const std::wstring& GetVSpath() { return currVSpath; }

	void SetPS(const wchar_t* path);
	inline const std::wstring& GetPSpath() { return currPSpath; }

private:
	std::wstring currVSpath;
	std::wstring currPSpath;

protected:
	MeshDrawCommand				meshDrawCommand;
	MaterialAsset               materialAsset;
	ConstantBuffer				material;
	ConstantBuffer				transformBuffer;

private:
	void CreateMesh();

private:
	std::vector<Vector4> vertices;
	std::vector<UINT> indices;
};