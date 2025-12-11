#pragma once

#include "RendererCore.h"
#include "Renderer.h"
#include "Light.h"
#include <vector>

#include <chrono>

#include <array>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/CommonStates.h>
#include <directxtk/VertexTypes.h>
#include <directxtk/Effects.h>
#include <directxtk/SpriteBatch.h>
#include <directxtk/SpriteFont.h>


struct FrameBufferData
{
	float Time;
	float Time0_1;
	float deltaTime;
	float pad2;
};

struct MeshDrawCommand2
{
public:
	RendererBuffer vertexBuffer;
	RendererBuffer indexBuffer;
	uint32_t indexCounts;
	uint32_t vertexStride;
	VertexShader vertexShader;
	int vsShaderResourcesStart;
	int vsShaderResourcesEnd;

	DirectX::BoundingOrientedBox boundingBox;


	PixelShader pixelShader;
	int psShaderResourcesStart;
	int psShaderResourcesEnd;
};

struct PostProcesCommand2
{
	int shaderResourcesStart;
	int shaderResourcesEnd;

	int computeShaderStart;
	int computeShaderEnd;

	bool isMipMap;
};


struct ParticleDrawCommand2
{
	PixelShader pixelShader;
	StructuredBuffer particleBuffer{};
	StructuredBuffer addParticleBuffer[2]{};
	StructuredBuffer deadParticleBuffer{};

	int addParticleCount;
	ConstantBuffer optionBuffer;

	RendererBuffer instanceCountBuffer{};
	RendererBuffer instanceCountStagingBuffer{};
	RendererBuffer deadParticleStagingBuffer{};
	int psShaderResourcesStart;
	int psShaderResourcesEnd;
	uint32_t instanceCount;
};

struct UIMaterialDrawCommand2 
{
	ConstantBuffer transformBuffer;
	PixelShader pixelShader;
	int shaderResourcesStart;
	int shaderResourcesEnd;
	float drawSpeed;
};

struct TextDrawCommand
{
	std::wstring text;
	Color color;
	float size_mult{ 1 };
	float x{ 0 };
	float y{ 0 };
	float drawSpeed;

	DirectX::SpriteFont* sprite_font{ nullptr };
};


// UI 스텐실 마스킹
struct StencilMaskingCommand
{
	ConstantBuffer transformBuffer;
	std::vector<Binadble> shaderResources;
	PixelShader pixelShader;
	float drawSpeed;
};
struct DrawingOnMaskingCommand
{
	ConstantBuffer transformBuffer;
	std::vector<Binadble> shaderResources;
	PixelShader pixelShader;
	float drawSpeed;
	/** 마스킹 되지않은부분을 그리겟다*/
	bool isdontMaskingDraw{false};
};
struct StencilMaskingCommand2
{
	ConstantBuffer transformBuffer;
	PixelShader pixelShader;
	int shaderResourcesStart;
	int shaderResourcesEnd;
	float drawSpeed;
};
struct DrawingOnMaskingCommand2
{
	ConstantBuffer transformBuffer;
	PixelShader pixelShader;
	int shaderResourcesStart;
	int shaderResourcesEnd;
	float drawSpeed;
	bool isdontMaskingDraw;
};
// UIRender1용 마스킹
struct UIRender1StencilMaskingCommand
{
	Texture texture;
	ConstantBuffer transformBuffer;
	ConstantBuffer colorBuffer;
	float drawSpeed;
};
struct UIRender1DrawingOnMaskingCommand
{
	Texture texture;
	ConstantBuffer transformBuffer;
	ConstantBuffer colorBuffer;
	float drawSpeed;
};




class DefferdRenderer : public IRenderer
{
public:
	DefferdRenderer();
	virtual ~DefferdRenderer();

public:
	void SetSkyBoxDrawCommand(_In_ const MeshDrawCommand& command);

	/*드로우 커맨드 초기화*/
	void ClearDrawCommands();
	
	virtual void AddDrawCommand(_In_ const MeshDrawCommand& command) override;
	void AddDrawCommand(_In_ const UIDrawCommand& command);
	void AddDrawCommand(_In_ const UIMaterialDrawCommand& command);
	void AddDrawCommand(_In_ const StencilMaskingCommand& command);
	void AddDrawCommand(_In_ const DrawingOnMaskingCommand& command);
	void AddDrawCommand(_In_ const UIRender1StencilMaskingCommand& command);
	void AddDrawCommand(_In_ const UIRender1DrawingOnMaskingCommand& command);
	void AddDrawCommand(_In_ const PostProcesCommand& command);
	void AddDrawCommand(_In_ const DebugMeshDrawCommand& command);
	void AddDrawCommand(_In_ const ParticleDrawCommand& command);
	void AddDrawCommand(_In_ const TextDrawCommand& command);
	virtual void AddBinadble(std::string_view key, const Binadble& bindable) override;
	virtual void RemoveBinadble(std::string_view key) override;
	virtual void SetRenderTarget(_In_ Texture& target) override;
	virtual void Render() override;

	virtual void SetCameraMatrix(const Matrix& world);
	virtual void SetPerspectiveProjection(float fov, float nearZ, float farZ);
	virtual void SetOrthographicProjection(float nearZ, float farZ);
	
	//마음에는 안들지만 방법이없음....
	ComputeShader deferredCS;
	PixelShader copyTexturePS;

	VertexShader fullScrennShader;
	VertexShader uiVertexShader;
	PixelShader uiPixelShader;


	VertexShader particleVertexShader;
	GeometryShader particleGeometryShader;
	ComputeShader particleComputeShader;

	Texture BRDF_LUT;
	Texture Diffuse_IBL;
	Texture Specular_IBL;
	
	PointLightBuffer pointLight;
	DirectionLightBuffer directLight;
	bool isWireFrame;
private:
	std::vector<MeshDrawCommand2> allDrawCommandsOrigin{};
	std::vector<MeshDrawCommand2*> allDrawCommands{};
	std::vector<MeshDrawCommand2*> deferredDrawCommands{};
	std::vector<MeshDrawCommand2*> forwardDrawCommands{};
	std::vector<MeshDrawCommand2*> alphaDrawCommands{};
	std::vector<Binadble> drawCommandBindable{};

	MeshDrawCommand skyBoxDrawCommand{};
	std::vector<UIDrawCommand> uiDrawCommands{};
	std::vector<UIMaterialDrawCommand2> uiDrawCommands2{};
	std::vector<TextDrawCommand> textDrawCommands{};

	std::vector<PostProcesCommand2> postProcesCommands{};
	std::vector<DebugMeshDrawCommand> debugDrawCommands{};
	std::vector<ParticleDrawCommand2> particleDrawCommands{};

	// 마스킹
	std::vector<StencilMaskingCommand2> uiStencilMaskingCommands{};
	std::vector<DrawingOnMaskingCommand2> uiDrawingOnMaskingCommands{};
	std::vector<UIRender1StencilMaskingCommand> render1uiStencilMaskingCommands{};
	std::vector<UIRender1DrawingOnMaskingCommand> render1uiDrawingOnMaskingCommands{};


	std::vector<std::function<void()>> computeShaderSets;
	std::vector<DispatchData> dispatchs;
	
	
	Texture renderTarget{};

	std::vector<std::string> bindablesKey;
	std::vector<Binadble> bindables;
	std::vector<ComputeShader> postProcesShaders{};

#pragma region RHIDevice

	ComPtr<ID3D11Device> device{};
	ComPtr<ID3D11DeviceContext> immediateContext{};

#pragma endregion RHIDevice


#pragma region RenderState

	ComPtr<struct ID3D11DepthStencilState> defaultDSS{};
	ComPtr<struct ID3D11DepthStencilState> noWriteDSS{};
	ComPtr<struct ID3D11BlendState> noRenderState{};
	ComPtr<struct ID3D11BlendState> alphaRenderState{};

#pragma endregion RenderState

#pragma region RenderTexture

	Texture depthStencilTexture{};
	std::array<Texture, 8> renderBuffers{};
	Texture deferredBuffer[2]{};
	//std::array<Texture, 2> PostProcessTexture;
	int renderBufferIndex{ 0 };


#pragma endregion RenderTexture


#pragma region ViewPort
	
	uint32_t width{ 0 };
	uint32_t height{ 0 };

#pragma endregion ViewPort


#pragma region Camera

	float fov{ 0.0f };
	float nearZ{ 0.0f };
	float farZ{ 0.0f };
	Matrix cameraWorld;
	Matrix cameraProjection;
	ConstantBuffer cameraBuffer;
	Binadble cameraBinadbleVS;
	Binadble cameraBinadblePS;
	Binadble cameraBinadbleCS;
	Binadble cameraBinadbleGS;

#pragma endregion Camera

	ComPtr<ID3D11SamplerState> samplerState[4];

#pragma region PerFrameConstants

	std::chrono::high_resolution_clock::time_point lastTime;
	ConstantBuffer frameBuffer;
	Binadble frameBufferPS;
	Binadble frameBufferCS;

#pragma endregion PerFrameConstants


#pragma region QuadMeshDraw

	RendererBuffer uiVertexBuffer;
	RendererBuffer uiIndexBuffer;
	uint32_t uiIndexCounts;
	uint32_t uiVertexStride;
	ComPtr<struct ID3D11RasterizerState> uiRasterizerState{};
	ComPtr<struct ID3D11DepthStencilState> uiDSS{};
	ComPtr<struct ID3D11DepthStencilState> uiStencilBuffer_onMasking{};
	ComPtr<struct ID3D11DepthStencilState> uiStencilBuffer_onDrawing{};
	ComPtr<struct ID3D11DepthStencilState> uiStencilBuffer_onDrawing2{};

#pragma endregion QuadMeshDraw


#pragma region DebugMeshDraw
	
	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	ComPtr<ID3D11InputLayout> m_layout;

#pragma endregion DebugMeshDraw


#pragma region TextDraw
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
public:
	std::unique_ptr<DirectX::SpriteFont> CreateSpriteFont(std::wstring_view path);
#pragma endregion TextDraw


private:
	void ProcessDrawCommand(MeshDrawCommand& drawCommands);
	void ProcessDrawCommand(UIDrawCommand& drawCommands);
	void ProcessDrawCommand(UIMaterialDrawCommand2& drawCommands);
	void ProcessDrawCommand(TextDrawCommand& drawCommands);
	void ProcessDrawCommand(PostProcesCommand2& drawCommands);
	void ProcessDrawCommand(ParticleDrawCommand2& drawCommands);
	void ProcessDrawCommands(std::vector<MeshDrawCommand2*>& drawCommands, bool isWithMaterial = true);
	void BindBinadble(const Binadble& bindable, bool unBind = false);


	void ProcessDrawCommand(StencilMaskingCommand2& drawCommands);
	void ProcessDrawCommand(DrawingOnMaskingCommand2& drawCommands);
	void ProcessDrawCommand(UIRender1StencilMaskingCommand& drawCommands);
	void ProcessDrawCommand(UIRender1DrawingOnMaskingCommand& drawCommands);
};

