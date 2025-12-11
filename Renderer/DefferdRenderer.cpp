#include "DefferdRenderer.h"
#include <format>
#include <functional>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <d3d11.h>
#include <DirectXMath.h>
#include "DebugDraw.h"
#pragma comment(lib, "d3d11.lib")


#ifdef _DEBUG
#include <dxgidebug.h>
//https://develop-dream.tistory.com/141
void list_remaining_d3d_objects()
{
	HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
	decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));

	IDXGIDebug* debug;

	GetDebugInterface(IID_PPV_ARGS(&debug));

	OutputDebugStringW(L"Starting Live Direct3D Object Dump:\r\n");
	debug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_DETAIL);
	OutputDebugStringW(L"Completed Live Direct3D Object Dump.\r\n");

	debug->Release();
}
#endif

#pragma comment(lib, "dxguid.lib")


DefferdRenderer::DefferdRenderer()
{
	HRESULT result;
	device = RendererUtility::GetDevice();
	if (!device)
	{
		UINT creationFlags = 0;
#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		result = D3D11CreateDevice(nullptr,
								   D3D_DRIVER_TYPE_HARDWARE,
								   nullptr,
								   creationFlags,
								   nullptr,
								   0,
								   D3D11_SDK_VERSION,
								   &device,
								   nullptr,
								   &immediateContext);
		Check(result);
	}
	RendererUtility::SetDevice(device);

	device->GetImmediateContext(&immediateContext);

	struct Init
	{
		ComPtr<ID3D11Device> device;

		void CreateDefaultDepthStencilState(ID3D11DepthStencilState** depthState)
		{
			CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
			desc.StencilEnable = true;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;

			HRESULT result;
			result = device->CreateDepthStencilState(&desc, depthState);
			Check(result);
		}
		void CreateNoWriteDepthStencilState(ID3D11DepthStencilState** depthState)
		{
			CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

			HRESULT result;
			result = device->CreateDepthStencilState(&desc, depthState);
			Check(result);
		}
		void CreateNoRenderState(ID3D11BlendState** depthState)
		{
			CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
			desc.RenderTarget[0].RenderTargetWriteMask = 0;

			HRESULT result;
			result = device->CreateBlendState(&desc, depthState);
			Check(result);
		}
		void CreateAlphaRenderState(ID3D11BlendState** depthState)
		{
			CD3D11_BLEND_DESC desc(D3D11_DEFAULT);

			desc.IndependentBlendEnable = true;
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

			desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL ^ D3D11_COLOR_WRITE_ENABLE_ALPHA;

			HRESULT result;
			result = device->CreateBlendState(&desc, depthState);
			Check(result);
		}
		void CreateUIMesh(RendererBuffer& vertexBuffer, RendererBuffer& indexBuffer, uint32_t& indexCount, uint32_t& vertexStride)
		{
			// quad메쉬 만들기
			struct Vertex
			{
				DirectX::XMFLOAT4 position;
				DirectX::XMFLOAT2 uv;
			};

			std::array<Vertex, 4> vertices
			{
				Vertex{ { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
				Vertex{ { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
				Vertex{ { -1.0f, -1.0f, 0.0f, 1.0f}, { 0.0f, 0.0f } },
				Vertex{ { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }
			};

			std::array<uint16_t, 6> indices
			{
				0, 1, 2,
				2, 1, 3
			};

			vertexStride = sizeof(Vertex);

			vertexBuffer.Init(D3D11_BUFFER_DESC
							  { 
								  .ByteWidth = sizeof(vertices), 
								  .Usage = D3D11_USAGE_IMMUTABLE, 
								  .BindFlags = D3D11_BIND_VERTEX_BUFFER, 
								  .CPUAccessFlags = 0 
							  }, 
							  sizeof(Vertex) * std::size(vertices),
							  vertices.data());
			indexBuffer.Init(D3D11_BUFFER_DESC
							 { 
								 .ByteWidth = sizeof(indices), 
								 .Usage = D3D11_USAGE_IMMUTABLE, 
								 .BindFlags = D3D11_BIND_INDEX_BUFFER, 
								 .CPUAccessFlags = 0 
							 }, 
							 sizeof(uint16_t)* std::size(indices),
							 indices.data());

			indexCount = indices.size();
		}
		void CreateUIDepthState(ID3D11DepthStencilState** dss, ID3D11RasterizerState** rs)
		{
			CD3D11_RASTERIZER_DESC RSdesc(D3D11_DEFAULT);
			RSdesc.CullMode = D3D11_CULL_NONE;

			CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
			desc.DepthEnable = false;

			HRESULT result;
			result = device->CreateDepthStencilState(&desc, dss);
			result = device->CreateRasterizerState(&RSdesc, rs);
			Check(result);
		}
		void CreateUIStencilBufferOnMasking(ID3D11DepthStencilState** dss)
		{
			D3D11_DEPTH_STENCIL_DESC desc{};
			desc.DepthEnable = true;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			desc.StencilEnable = true;
			desc.StencilReadMask = 0xFF;
			desc.StencilWriteMask = 0xFF;
			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			desc.BackFace = desc.FrontFace;

			HRESULT result = device->CreateDepthStencilState(&desc, dss);
			Check(result);
		}
		void CreateUIStencilBufferOnDrawing(ID3D11DepthStencilState** dss,ID3D11DepthStencilState** dss2)
		{
			D3D11_DEPTH_STENCIL_DESC desc{};
			desc.DepthEnable = true;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			desc.StencilEnable = true;
			desc.StencilReadMask = 0xFF;
			desc.StencilWriteMask = 0xFF;
			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			desc.BackFace = desc.FrontFace;

			HRESULT result = device->CreateDepthStencilState(&desc, dss);
			Check(result);

			desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
			desc.BackFace = desc.FrontFace;
			result = device->CreateDepthStencilState(&desc, dss2);
			Check(result);
		}
		void CreateDebugObject(std::unique_ptr<DirectX::CommonStates>& m_states, 
							   std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>& m_batch,
							   std::unique_ptr<DirectX::BasicEffect>& m_effect,
							   ComPtr<ID3D11InputLayout>& m_layout)
		{
			HRESULT result;
			using namespace DirectX;

			ComPtr<ID3D11DeviceContext> immediateContext{};
			device->GetImmediateContext(&immediateContext);

			m_states = std::make_unique<CommonStates>(device.Get());
			m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(immediateContext.Get());

			m_effect = std::make_unique<BasicEffect>(device.Get());
			m_effect->SetVertexColorEnabled(true);
			void const* shaderByteCode;
			size_t byteCodeLength;
			m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

			result = device->CreateInputLayout(
				VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
				shaderByteCode, byteCodeLength,
				m_layout.ReleaseAndGetAddressOf());
			Check(result);
		}
	};

	Init init{ device };
	init.CreateDefaultDepthStencilState(&defaultDSS);
	init.CreateNoWriteDepthStencilState(&noWriteDSS);
	init.CreateNoRenderState(&noRenderState);
	init.CreateAlphaRenderState(&alphaRenderState);
	init.CreateUIMesh(uiVertexBuffer, uiIndexBuffer, uiIndexCounts, uiVertexStride);
	init.CreateUIDepthState(&uiDSS, &uiRasterizerState);
	init.CreateUIStencilBufferOnMasking(&uiStencilBuffer_onMasking);
	init.CreateUIStencilBufferOnDrawing(&uiStencilBuffer_onDrawing, &uiStencilBuffer_onDrawing2);
	init.CreateDebugObject(m_states, m_batch, m_effect, m_layout);
	spriteBatch = std::make_unique<DirectX::SpriteBatch>(immediateContext.Get());

	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	device->CreateSamplerState(&samplerDesc, &samplerState[0]);

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&samplerDesc, &samplerState[1]);

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	device->CreateSamplerState(&samplerDesc, &samplerState[2]);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	device->CreateSamplerState(&samplerDesc, &samplerState[3]);

	
	CameraBufferData cameraData{};
	cameraBuffer.Init(cameraData);
	cameraBinadbleVS.shaderType = EShaderType::Vertex;
	cameraBinadbleVS.bindableType = EShaderBindable::ConstantBuffer;
	cameraBinadbleVS.slot = 1;
	cameraBinadbleVS.bind = cameraBuffer;

	cameraBinadblePS.shaderType = EShaderType::Pixel;
	cameraBinadblePS.bindableType = EShaderBindable::ConstantBuffer;
	cameraBinadblePS.slot = 1;
	cameraBinadblePS.bind = cameraBuffer;

	cameraBinadbleCS.shaderType = EShaderType::Compute;
	cameraBinadbleCS.bindableType = EShaderBindable::ConstantBuffer;
	cameraBinadbleCS.slot = 1;
	cameraBinadbleCS.bind = cameraBuffer;

	cameraBinadbleGS.shaderType = EShaderType::Geometry;
	cameraBinadbleGS.bindableType = EShaderBindable::ConstantBuffer;
	cameraBinadbleGS.slot = 1;
	cameraBinadbleGS.bind = cameraBuffer;

	FrameBufferData frameData{};
	frameBuffer.Init(frameData);
	frameBufferPS.shaderType = EShaderType::Pixel;
	frameBufferPS.bindableType = EShaderBindable::ConstantBuffer;
	frameBufferPS.slot = 3;
	frameBufferPS.bind = frameBuffer;

	frameBufferCS.shaderType = EShaderType::Compute;
	frameBufferCS.bindableType = EShaderBindable::ConstantBuffer;
	frameBufferCS.slot = 3;
	frameBufferCS.bind = frameBuffer;
}

DefferdRenderer::~DefferdRenderer()
{
	RendererUtility::SetDevice(nullptr);
#ifdef _DEBUG
	atexit(list_remaining_d3d_objects);
#endif
}

void DefferdRenderer::SetSkyBoxDrawCommand(_In_ const MeshDrawCommand& command)
{
	skyBoxDrawCommand = command;
}

void DefferdRenderer::ClearDrawCommands()
{
	drawCommandBindable.clear();
	allDrawCommandsOrigin.clear();
	uiDrawCommands.clear();
	textDrawCommands.clear();
}

void DefferdRenderer::AddDrawCommand(_In_ const MeshDrawCommand& command)
{
	int vsShaderResourcesStart = drawCommandBindable.size();
	{
		std::ranges::copy(command.meshData.shaderResources, std::back_inserter(drawCommandBindable));
	}
	int vsShaderResourcesEnd = drawCommandBindable.size();
	int psShaderResourcesStart = vsShaderResourcesEnd;
	{
		std::ranges::copy(command.materialData.shaderResources, std::back_inserter(drawCommandBindable));
	}
	int psShaderResourcesEnd = drawCommandBindable.size();
	
	allDrawCommandsOrigin.emplace_back(MeshDrawCommand2
									   {
										   command.meshData.vertexBuffer,
										   command.meshData.indexBuffer,
										   command.meshData.indexCounts,
										   command.meshData.vertexStride,
										   command.meshData.vertexShader,
										   vsShaderResourcesStart,
										   vsShaderResourcesEnd,
										   command.meshData.boundingBox,
										   command.materialData.pixelShader,
										   psShaderResourcesStart,
										   psShaderResourcesEnd
									   });
	
}

void DefferdRenderer::AddDrawCommand(_In_ const UIDrawCommand& command)
{
	uiDrawCommands.emplace_back(command);
}

void DefferdRenderer::AddDrawCommand(_In_ const UIMaterialDrawCommand& command)
{
	UIMaterialDrawCommand2 uiCommand{};
	uiCommand.transformBuffer = command.transformBuffer;
	uiCommand.pixelShader = command.pixelShader;
	uiCommand.shaderResourcesStart = drawCommandBindable.size();
	{
		std::ranges::copy(command.shaderResources, std::back_inserter(drawCommandBindable));
	}
	uiCommand.shaderResourcesEnd = drawCommandBindable.size();
	uiCommand.drawSpeed = command.drawSpeed;
	uiDrawCommands2.emplace_back(uiCommand);
}

void DefferdRenderer::AddDrawCommand(_In_ const StencilMaskingCommand& command)
{
	StencilMaskingCommand2 uiCommand{};
	uiCommand.transformBuffer = command.transformBuffer;
	uiCommand.pixelShader = command.pixelShader;
	uiCommand.shaderResourcesStart = drawCommandBindable.size();
	{
		std::ranges::copy(command.shaderResources, std::back_inserter(drawCommandBindable));
	}
	uiCommand.shaderResourcesEnd = drawCommandBindable.size();
	uiCommand.drawSpeed = command.drawSpeed;

	uiStencilMaskingCommands.push_back(uiCommand);
}

void DefferdRenderer::AddDrawCommand(_In_ const DrawingOnMaskingCommand& command)
{
	DrawingOnMaskingCommand2 uiCommand{};
	uiCommand.transformBuffer = command.transformBuffer;
	uiCommand.pixelShader = command.pixelShader;
	uiCommand.shaderResourcesStart = drawCommandBindable.size();
	{
		std::ranges::copy(command.shaderResources, std::back_inserter(drawCommandBindable));
	}
	uiCommand.shaderResourcesEnd = drawCommandBindable.size();
	uiCommand.drawSpeed = command.drawSpeed; 
	uiCommand.isdontMaskingDraw = command.isdontMaskingDraw;

	uiDrawingOnMaskingCommands.push_back(uiCommand);
}

void DefferdRenderer::AddDrawCommand(_In_ const UIRender1StencilMaskingCommand& command)
{
	render1uiStencilMaskingCommands.push_back(command);
}

void DefferdRenderer::AddDrawCommand(_In_ const UIRender1DrawingOnMaskingCommand& command)
{
	render1uiDrawingOnMaskingCommands.push_back(command);
}


void DefferdRenderer::AddDrawCommand(_In_ const PostProcesCommand& command)
{
	int shaderResourcesStart = drawCommandBindable.size();
	{
		std::ranges::copy(command.shaderResources, std::back_inserter(drawCommandBindable));
	}
	int shaderResourcesEnd = drawCommandBindable.size();

	int computeShaderStart = postProcesShaders.size();
	{
		std::ranges::copy(command.computeShader, std::back_inserter(postProcesShaders));
		std::ranges::copy(command.computeShaderSet, std::back_inserter(computeShaderSets));
		std::ranges::copy(command.dispatchDatas, std::back_inserter(dispatchs));
	}
	int computeShaderEnd = postProcesShaders.size();



	PostProcesCommand2 postProcesCommand{};
	postProcesCommand.shaderResourcesStart = shaderResourcesStart;
	postProcesCommand.shaderResourcesEnd = shaderResourcesEnd;
	postProcesCommand.computeShaderStart = computeShaderStart;
	postProcesCommand.computeShaderEnd = computeShaderEnd;

	postProcesCommand.isMipMap = command.isMipMap;
	postProcesCommands.emplace_back(postProcesCommand);
}

void DefferdRenderer::AddDrawCommand(_In_ const DebugMeshDrawCommand& command)
{
	debugDrawCommands.emplace_back(command);
}

void DefferdRenderer::AddDrawCommand(_In_ const ParticleDrawCommand& command)
{
	ParticleDrawCommand2 particleCommand{};
	particleCommand.pixelShader = command.pixelShader;
	particleCommand.particleBuffer = command.particleBuffer;
	
	particleCommand.addParticleBuffer[0] = command.addParticleBuffer[0];
	particleCommand.addParticleBuffer[1] = command.addParticleBuffer[1];

	particleCommand.addParticleCount = command.addParticleCount;
	particleCommand.deadParticleBuffer = command.deadParticleBuffer;

	particleCommand.instanceCount = command.instanceCount;

	particleCommand.optionBuffer = command.optionBuffer;
	particleCommand.instanceCountBuffer = command.instanceCountBuffer;
	particleCommand.instanceCountStagingBuffer = command.instanceCountStagingBuffer;
	particleCommand.deadParticleStagingBuffer = command.deadParticleStagingBuffer;

	particleCommand.psShaderResourcesStart = drawCommandBindable.size();
	{
		std::ranges::copy(command.PSshaderResources, std::back_inserter(drawCommandBindable));
	}
	particleCommand.psShaderResourcesEnd = drawCommandBindable.size();

	particleDrawCommands.emplace_back(particleCommand);
}

void DefferdRenderer::AddDrawCommand(_In_ const TextDrawCommand& command)
{
	textDrawCommands.emplace_back(command);
}

void DefferdRenderer::AddBinadble(std::string_view key, const Binadble& bindable)
{
	bindablesKey.emplace_back(key);
	bindables.emplace_back(bindable);
}

void DefferdRenderer::RemoveBinadble(std::string_view key)
{
	auto iter = std::find(bindablesKey.begin(), bindablesKey.end(), key);
	if (iter != bindablesKey.end())
	{
		auto index = std::distance(bindablesKey.begin(), iter);
		bindables.erase(bindables.begin() + index);
		bindablesKey.erase(iter);
	}
}

void DefferdRenderer::SetRenderTarget(_In_ Texture& target)
{
	renderTarget = target;

	// 길이 초기화
	{
		D3D11_TEXTURE2D_DESC desc{};
		((ID3D11Texture2D*)target)->GetDesc(&desc);
		width = desc.Width;
		height = desc.Height;
	}

	// G버퍼
	for (size_t i = 0; i < std::size(renderBuffers); i++)
	{
		HRESULT result;
		ComPtr<ID3D11Texture2D> texture;
		ETextureUsage::Type usage = ETextureUsage::SRV | ETextureUsage::RTV;

		CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC
		{ 
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			width,
			height,
			1,
			1,
			usage 
		};


		result = device->CreateTexture2D(&desc, nullptr, &texture);
		Check(result);
		renderBuffers[i].LoadTexture(texture.Get(), usage);
	}

	// 깊이버퍼
	{
		HRESULT result;
		ComPtr<ID3D11Texture2D> texture;
		ETextureUsage::Type usage = ETextureUsage::SRV | ETextureUsage::DSV;

		CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC
		{
			DXGI_FORMAT_R24G8_TYPELESS,
			width,
			height,
			1,
			1,
			usage 
		};

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC
		{ 
			D3D11_SRV_DIMENSION_TEXTURE2D,
			DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		};

		result = device->CreateTexture2D(&desc, nullptr, &texture);
		Check(result);
		depthStencilTexture.LoadTexture(texture.Get(), usage, &srvDesc);
	}
	{
		HRESULT result;
		ComPtr<ID3D11Texture2D> texture;
		ETextureUsage::Type usage = ETextureUsage::RTV | ETextureUsage::UAV | ETextureUsage::SRV;
		CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC
		{
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			width,
			height,
			1,
			0,
			usage
		};
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		result = device->CreateTexture2D(&desc, nullptr, &texture);
		Check(result);
		deferredBuffer[0].LoadTexture(texture.Get(), usage);



		result = device->CreateTexture2D(&desc, nullptr, &texture);
		Check(result);
		deferredBuffer[1].LoadTexture(texture.Get(), usage);
	}
	// 후처리 텍스처
	/*for (size_t i = 0; i < std::size(PostProcessTexture); i++)
	{
		HRESULT result;
		ComPtr<ID3D11Texture2D> texture;
		ETextureUsage::Type usage = ETextureUsage::SRV | ETextureUsage::UAV;

		CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC
		{ 
			DXGI_FORMAT_R8G8B8A8_UNORM,
			width,
			height,
			1,
			1,
			usage
		};


		result = device->CreateTexture2D(&desc, nullptr, &texture);
		Check(result);
		PostProcessTexture[i].LoadTexture(texture.Get(), usage);
		
	}*/
	
}

void DefferdRenderer::Render()

{
	DirectX::BoundingFrustum frustum(cameraProjection);
	frustum.Transform(frustum, cameraWorld);

	auto culledDrawCommands =
		allDrawCommandsOrigin
		| std::views::filter([frustum](const MeshDrawCommand2& item) { return frustum.Intersects(item.boundingBox); })
		| std::views::transform([](MeshDrawCommand2& item) -> MeshDrawCommand2* { return &item; });

	std::ranges::copy(culledDrawCommands, std::back_inserter(allDrawCommands));
	std::ranges::copy(culledDrawCommands | std::views::filter([](MeshDrawCommand2* item) { return item->pixelShader.isForward; }), std::back_inserter(forwardDrawCommands));
	std::ranges::copy(culledDrawCommands | std::views::filter([](MeshDrawCommand2* item) { return !item->pixelShader.isForward; }), std::back_inserter(deferredDrawCommands));

	auto boundingBoxs =
		culledDrawCommands
		| std::views::transform([](MeshDrawCommand2* item) { return item->boundingBox; })
		| std::views::filter([frustum](const BoundingOrientedBox& item) { return frustum.Intersects(item); });

	auto visibilityBox = std::accumulate(boundingBoxs.begin(), boundingBoxs.end(),
										 BoundingBox(XMFLOAT3(cameraProjection.Translation()), XMFLOAT3(20.0f, 20.f, 20.f)),
										 [](const BoundingBox& a, const BoundingOrientedBox& b)
										 {
											 XMFLOAT3 boxCorners[8];
											 b.GetCorners(boxCorners);
											 BoundingBox tempBoundingBox;
											 BoundingBox::CreateFromPoints(tempBoundingBox, std::size(boxCorners), std::data(boxCorners), sizeof(XMFLOAT3));
											 BoundingBox box;
											 BoundingBox::CreateMerged(box, a, tempBoundingBox);
											 return box;
										 });

	immediateContext->OMSetDepthStencilState(nullptr, 0);

	ID3D11ShaderResourceView* nullSRVPtr[1] = { nullptr };
	immediateContext->PSSetShaderResources(20, 1, nullSRVPtr);
	immediateContext->CSSetShaderResources(20, 1, nullSRVPtr);

	// 쉐도우맵 생성
	for (size_t i = 0; i < directLight.size(); i++)
	{
		DirectionLightData& lightData = directLight.GetDirectLight(i);

		XMMATRIX view;
		XMMATRIX projection;
		DirectionLightBuffer::ComputeLightMatrix(visibilityBox,
												 Vector3(lightData.Directoin),
												 view,
												 projection);
		CameraBufferData cameraData{};
		cameraData.View = DirectX::XMMatrixTranspose(view);
		cameraData.Projection = DirectX::XMMatrixTranspose(projection);
		directLight.GetLightCamera(i).Set(cameraData);

		Matrix VPMatrix = view * projection;
		lightData.VP = DirectX::XMMatrixTranspose(VPMatrix);

		D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)directLight.GetTextureSize(), (float)directLight.GetTextureSize());
		ID3D11Buffer* cameraBufferPtr[1] = { (ID3D11Buffer*)directLight.GetLightCamera(i) };

		immediateContext->OMSetRenderTargets(0, { nullptr }, directLight.GetShadowMapDS(i));
		immediateContext->ClearDepthStencilView(directLight.GetShadowMapDS(i), D3D11_CLEAR_DEPTH, 1.0f, 0);

		immediateContext->OMSetBlendState(noRenderState.Get(), nullptr, 0xffffffff);
		immediateContext->PSSetShader(nullptr, nullptr, 0);
		immediateContext->RSSetViewports(1, &viewport);
		immediateContext->VSSetConstantBuffers(1, std::size(cameraBufferPtr), cameraBufferPtr);

		ProcessDrawCommands(deferredDrawCommands, false);
		immediateContext->OMSetRenderTargets(0, { nullptr }, nullptr);
	}
	


	// 데이터 바인딩
	{
		D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)width, (float)height);
		ID3D11RenderTargetView* renderBuffersRTV[8];
		for (size_t i = 0; i < std::size(renderBuffersRTV); i++)
		{
			renderBuffersRTV[i] = renderBuffers[i];
		}

		immediateContext->OMSetRenderTargets(std::size(renderBuffersRTV), renderBuffersRTV, depthStencilTexture);
		immediateContext->RSSetViewports(1, &viewport);

		for (size_t i = 0; i < std::size(renderBuffers); i++)
		{
			immediateContext->ClearRenderTargetView(renderBuffers[i], DirectX::SimpleMath::Color{ 0.0f, 0.0f, 0.0f, 0.0f });
		}
		immediateContext->ClearDepthStencilView(depthStencilTexture, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		immediateContext->ClearRenderTargetView(deferredBuffer[0], DirectX::SimpleMath::Color{0.0f, 0.0f, 0.0f, 0.0f});


		CameraBufferData cameraData{};
		cameraData.MainCamPos = cameraWorld.Translation();
		cameraData.IPM = DirectX::XMMatrixTranspose(cameraProjection.Invert());
		cameraData.IVM = DirectX::XMMatrixTranspose(cameraWorld);
		cameraData.Projection = DirectX::XMMatrixTranspose(cameraProjection);
		cameraData.View = DirectX::XMMatrixTranspose(cameraWorld.Invert());

		cameraBuffer.Update(cameraData);
		BindBinadble(cameraBinadbleVS);
		BindBinadble(cameraBinadblePS);
		BindBinadble(cameraBinadbleCS);
		BindBinadble(cameraBinadbleGS);


		auto currTime = std::chrono::high_resolution_clock::now();

		FrameBufferData frameData{};
		frameData.Time = std::chrono::duration<float>(currTime.time_since_epoch()).count();
		frameData.Time0_1 = frameData.Time - std::floor(frameData.Time);
		frameData.deltaTime = std::chrono::duration<float>(currTime - lastTime).count();
		lastTime = currTime;

		frameBuffer.Update(frameData);
		BindBinadble(frameBufferPS);
		BindBinadble(frameBufferCS);

		ID3D11SamplerState* samplerStatePtr[4] = { samplerState[0].Get(), samplerState[1].Get(), samplerState[2].Get(), samplerState[3].Get() };
		immediateContext->PSSetSamplers(0, std::size(samplerStatePtr), std::data(samplerStatePtr));
		immediateContext->CSSetSamplers(0, std::size(samplerStatePtr), std::data(samplerStatePtr));

		directLight.UpdateBuffer();
		pointLight.UpdateBuffer();

		ID3D11ShaderResourceView* ShadowMapPtr[1] = { directLight.GetShadowMapArray() };
		immediateContext->PSSetShaderResources(20, 1, ShadowMapPtr);
		immediateContext->CSSetShaderResources(20, 1, ShadowMapPtr);

		ID3D11ShaderResourceView* LightBufferPtr[1] = { directLight.GetDirectLightBuffer() };
		immediateContext->PSSetShaderResources(16, 1, LightBufferPtr);
		immediateContext->CSSetShaderResources(16, 1, LightBufferPtr);

		ID3D11ShaderResourceView* pointLightBufferPtr[1] = { pointLight.GetPointLightBuffer() };
		immediateContext->PSSetShaderResources(17, 1, pointLightBufferPtr);
		immediateContext->CSSetShaderResources(17, 1, pointLightBufferPtr);


		for (auto& item : bindables)
		{
			BindBinadble(item);
		}
	}
	if (isWireFrame)
	{
		immediateContext->RSSetState(m_states->Wireframe());
	}
	else
	{
		immediateContext->RSSetState(m_states->CullCounterClockwise());
	}
	// Gbuffer 기록
	{
		immediateContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		immediateContext->OMSetDepthStencilState(defaultDSS.Get(), 0);
		ProcessDrawCommands(deferredDrawCommands);
	}

	// Gbuffer 라이팅 처리
	{
		ID3D11RenderTargetView* nullRenderBuffersRTV[8]{ nullptr, };
		ID3D11ShaderResourceView* renderBuffersSRV[8];
		ID3D11ShaderResourceView* depthBuffersSRV[1] = { depthStencilTexture };
		ID3D11UnorderedAccessView* deferredBufferUAV[1] = { deferredBuffer[0]};
		std::ranges::copy(renderBuffers, renderBuffersSRV);


		immediateContext->ClearRenderTargetView(deferredBuffer[0], DirectX::SimpleMath::Color{ 0.0f, 0.0f , 0.0f , 0.0f });
		immediateContext->ClearRenderTargetView(deferredBuffer[1], DirectX::SimpleMath::Color{ 0.0f, 0.0f , 0.0f , 0.0f });
		immediateContext->CSSetShader(deferredCS, nullptr, 0);
		immediateContext->OMSetRenderTargets(std::size(nullRenderBuffersRTV), nullRenderBuffersRTV, nullptr);
		immediateContext->CSSetShaderResources(0, std::size(renderBuffersSRV), renderBuffersSRV);
		immediateContext->CSSetShaderResources(8, std::size(depthBuffersSRV), depthBuffersSRV);
		immediateContext->CSSetUnorderedAccessViews(0, std::size(deferredBufferUAV), deferredBufferUAV, nullptr);

		immediateContext->Dispatch(width / 64 + 1, height, 1);

		ID3D11ShaderResourceView* nullSRV[9]{};
		ID3D11UnorderedAccessView* nullUAV[1]{};
		immediateContext->CSSetShaderResources(0, std::size(nullSRV), nullSRV);
		immediateContext->CSSetUnorderedAccessViews(0, std::size(nullUAV), nullUAV, nullptr);
	}

	// 디퍼드후 포워드 렌더
	{
		ID3D11RenderTargetView* deferredBufferRTV[5]{ deferredBuffer[0], renderBuffers[4], renderBuffers[5],renderBuffers[6], renderBuffers[7] };
		immediateContext->OMSetRenderTargets(std::size(deferredBufferRTV), deferredBufferRTV, depthStencilTexture);


		immediateContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		immediateContext->OMSetDepthStencilState(defaultDSS.Get(), 0);

		immediateContext->OMSetBlendState(alphaRenderState.Get(), nullptr, 0xffffffff);
		immediateContext->OMSetDepthStencilState(noWriteDSS.Get(), 0);
		ProcessDrawCommands(forwardDrawCommands);

		immediateContext->OMSetBlendState(alphaRenderState.Get(), nullptr, 0xffffffff);
		immediateContext->OMSetDepthStencilState(noWriteDSS.Get(), 0);
		ProcessDrawCommands(alphaDrawCommands);

		immediateContext->RSSetState(uiRasterizerState.Get());
		std::ranges::for_each(particleDrawCommands, [this](auto& item) { ProcessDrawCommand(item); });
		
		immediateContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		immediateContext->OMSetDepthStencilState(defaultDSS.Get(), 0);

		//ProcessDrawCommand(skyBoxDrawCommand);

		ID3D11RenderTargetView* nullSRV[5]{ nullptr };
		immediateContext->OMSetRenderTargets(std::size(nullSRV), nullSRV, nullptr);
	}

	// 후처리
	{
		int currentIndex = 0;
		ID3D11UnorderedAccessView* deferredBufferUAV[1] = { deferredBuffer[1] };
		ID3D11ShaderResourceView* deferredBufferSRV[1] = { deferredBuffer[0] };

		ID3D11ShaderResourceView* renderBuffersSRV[8];
		ID3D11ShaderResourceView* DSSRV[1] = { depthStencilTexture };
		std::ranges::copy(renderBuffers, renderBuffersSRV);


		immediateContext->CSSetShaderResources(0, std::size(renderBuffersSRV), renderBuffersSRV);
		immediateContext->CSSetShaderResources(8, 1, DSSRV);
		immediateContext->CSSetShaderResources(9, std::size(deferredBufferSRV), deferredBufferSRV);
		immediateContext->CSSetUnorderedAccessViews(0, std::size(deferredBufferUAV), deferredBufferUAV, nullptr);

		for (auto& item : postProcesCommands)
		{
			ProcessDrawCommand(item);
			std::swap(deferredBuffer[0], deferredBuffer[1]); 
			
			deferredBufferUAV[0] = { nullptr };
			deferredBufferSRV[0] = { nullptr };
			immediateContext->CSSetShaderResources(9, std::size(deferredBufferSRV), deferredBufferSRV);
			immediateContext->CSSetUnorderedAccessViews(0, std::size(deferredBufferUAV), deferredBufferUAV, nullptr);

			deferredBufferUAV[0] = { deferredBuffer[1] };
			deferredBufferSRV[0] = { deferredBuffer[0] };
			immediateContext->CSSetShaderResources(9, std::size(deferredBufferSRV), deferredBufferSRV);
			immediateContext->CSSetUnorderedAccessViews(0, std::size(deferredBufferUAV), deferredBufferUAV, nullptr);
		}

		ID3D11ShaderResourceView* nullSRV[10]{};
		ID3D11UnorderedAccessView* nullUAV[1]{};
		immediateContext->CSSetShaderResources(0, std::size(nullSRV), nullSRV);
		immediateContext->CSSetUnorderedAccessViews(0, std::size(nullUAV), nullUAV, nullptr);
	}





	// UI & Text 렌더
	{
		immediateContext->RSSetState(uiRasterizerState.Get());
		ID3D11RenderTargetView* deferredBufferRTV[1]{ deferredBuffer[0] };
		immediateContext->OMSetRenderTargets(std::size(deferredBufferRTV), deferredBufferRTV, nullptr);
		immediateContext->OMSetBlendState(alphaRenderState.Get(), nullptr, 0xffffffff);
		immediateContext->OMSetDepthStencilState(uiDSS.Get(), 0);

		ID3D11SamplerState* samplerStatePtr[4] = { samplerState[0].Get(), samplerState[1].Get(), samplerState[2].Get(), samplerState[3].Get() };
		immediateContext->PSSetSamplers(0, std::size(samplerStatePtr), std::data(samplerStatePtr));
		  
		std::sort(uiDrawCommands.begin(), uiDrawCommands.end(), [](UIDrawCommand& a, UIDrawCommand& b)
				  {
					  if (a.drawSpeed == b.drawSpeed)
					  {
						  return &a > &b;
					  }
					  else
					  {
						  return a.drawSpeed < b.drawSpeed;
					  }
				  });
		std::sort(uiDrawCommands2.begin(), uiDrawCommands2.end(), [](UIMaterialDrawCommand2& a, UIMaterialDrawCommand2& b)
				  {
					  if (a.drawSpeed == b.drawSpeed)
					  {
						  return &a > &b;
					  }
					  else
					  {
						  return a.drawSpeed < b.drawSpeed;
					  }
				  });
		std::sort(textDrawCommands.begin(), textDrawCommands.end(), [](TextDrawCommand& a, TextDrawCommand& b)
				  {
					  if (a.drawSpeed == b.drawSpeed)
					  {
						  return &a > &b;
					  }
					  else
					  {
						  return a.drawSpeed < b.drawSpeed;
					  }
				  });

		auto uiA = uiDrawCommands.begin();
		auto uiB = uiDrawCommands2.begin();
		auto txt = textDrawCommands.begin();
		while (uiA != uiDrawCommands.end() || uiB != uiDrawCommands2.end() || txt != textDrawCommands.end())
		{
			immediateContext->RSSetState(uiRasterizerState.Get());
			float min_speed = FLT_MAX;
			int cur_min_type = -1;

			if (uiA != uiDrawCommands.end() && (*uiA).drawSpeed < min_speed)
			{
				min_speed = (*uiA).drawSpeed;
				cur_min_type = 0;
			}
			if (uiB != uiDrawCommands2.end() && (*uiB).drawSpeed < min_speed)
			{
				min_speed = (*uiB).drawSpeed;
				cur_min_type = 1;
			}
			if (txt != textDrawCommands.end() && (*txt).drawSpeed < min_speed)
			{
				min_speed = (*txt).drawSpeed;
				cur_min_type = 2;
			}

			// 선택된 벡터에서 ProcessDrawCommand 호출 및 인덱스 증가
			if (cur_min_type == 0)
			{
				ProcessDrawCommand((*uiA));
				++uiA;
			}
			else if (cur_min_type == 1)
			{
				ProcessDrawCommand((*uiB));
				++uiB;
			}
			else if (cur_min_type == 2)
			{
				ProcessDrawCommand((*txt));
				immediateContext->OMSetBlendState(alphaRenderState.Get(), nullptr, 0xffffffff); //텍스트가 알파 바꿔버림;;
				++txt;
			}
		}


		ID3D11RenderTargetView* nullSRV[1]{ nullptr };
		immediateContext->OMSetRenderTargets(std::size(nullSRV), nullSRV, nullptr);
		immediateContext->RSSetState(uiRasterizerState.Get());
	}

	//immediateContext->CopyResource(renderTarget, PostProcessTexture[renderBufferIndex]);
	// 텍스처 복사
	{
		ID3D11RenderTargetView* renderTarget[1] = { this->renderTarget };
		ID3D11ShaderResourceView* deferredBufferSRV[1] = { deferredBuffer[0] };

		immediateContext->OMSetRenderTargets(std::size(renderTarget), renderTarget, nullptr);
		immediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		immediateContext->IASetIndexBuffer(0, (DXGI_FORMAT)0, 0);
		immediateContext->IASetInputLayout(fullScrennShader);
		immediateContext->VSSetShader(fullScrennShader, nullptr, 0);
		immediateContext->PSSetShader(copyTexturePS, nullptr, 0);


		immediateContext->PSSetShaderResources(0, std::size(deferredBufferSRV), deferredBufferSRV);
		immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		immediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		immediateContext->Draw(3, 0);

		ID3D11RenderTargetView* nullRTV[1]{ nullptr };
		immediateContext->OMSetRenderTargets(std::size(nullRTV), nullRTV, nullptr);

		ID3D11ShaderResourceView* nullSRV[1]{ nullptr };
		immediateContext->PSSetShaderResources(0, std::size(nullSRV), nullSRV);
	}

	// UI 마스킹
	{
		immediateContext->OMSetBlendState(alphaRenderState.Get(), nullptr, 0xffffffff);

		ID3D11RenderTargetView* _rtv[1] = { nullptr };
		immediateContext->OMSetRenderTargets(0, _rtv, depthStencilTexture);

		immediateContext->OMSetDepthStencilState(uiStencilBuffer_onMasking.Get(), 0xFF);

		for (auto& command : uiStencilMaskingCommands)
		{
			ProcessDrawCommand(command);
		}
		for (auto& command : render1uiStencilMaskingCommands)
		{
			ProcessDrawCommand(command);
		}

		ID3D11RenderTargetView* backBuffersRTV[1] = { renderTarget };
		immediateContext->OMSetRenderTargets(std::size(backBuffersRTV), backBuffersRTV, depthStencilTexture);


		std::sort(uiDrawingOnMaskingCommands.begin(), uiDrawingOnMaskingCommands.end(), [](DrawingOnMaskingCommand2& a, DrawingOnMaskingCommand2& b)
				  {
					  if (a.drawSpeed == b.drawSpeed)
					  {
						  return &a > &b;
					  }
					  else
					  {
						  return a.drawSpeed < b.drawSpeed;
					  }
				  });
		std::sort(render1uiDrawingOnMaskingCommands.begin(), render1uiDrawingOnMaskingCommands.end(), [](UIRender1DrawingOnMaskingCommand& a, UIRender1DrawingOnMaskingCommand& b)
				  {
					  if (a.drawSpeed == b.drawSpeed)
					  {
						  return &a > &b;
					  }
					  else
					  {
						  return a.drawSpeed < b.drawSpeed;
					  }
				  });

		auto _iter1 = uiDrawingOnMaskingCommands.begin();
		auto _iter2 = render1uiDrawingOnMaskingCommands.begin();
		while (_iter1 != uiDrawingOnMaskingCommands.end() || _iter2 != render1uiDrawingOnMaskingCommands.end())
		{
			float min_speed = FLT_MAX;
			int cur_min_type = -1;

			if (_iter1 != uiDrawingOnMaskingCommands.end() && (*_iter1).drawSpeed < min_speed)
			{
				min_speed = (*_iter1).drawSpeed;
				cur_min_type = 0;
			}
			if (_iter2 != render1uiDrawingOnMaskingCommands.end() && (*_iter2).drawSpeed < min_speed)
			{
				min_speed = (*_iter2).drawSpeed;
				cur_min_type = 1;
			}

			// 선택된 벡터에서 ProcessDrawCommand 호출 및 인덱스 증가
			if (cur_min_type == 0)
			{
				immediateContext->OMSetDepthStencilState(_iter1->isdontMaskingDraw ? uiStencilBuffer_onDrawing2.Get() : uiStencilBuffer_onDrawing.Get(), 0xFF);
				ProcessDrawCommand((*_iter1));
				++_iter1;
			}
			else if (cur_min_type == 1)
			{
				immediateContext->OMSetDepthStencilState(uiStencilBuffer_onDrawing.Get(), 0xFF);
				ProcessDrawCommand((*_iter2));
				++_iter2;
			}
		}
	}







	/// / 디버그 렌더
	{

		immediateContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
		immediateContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
		//immediateContext->RSSetState(m_states->CullNone());


		m_effect->SetView(cameraWorld.Invert());
		m_effect->SetProjection(cameraProjection);



		m_effect->Apply(immediateContext.Get());
		immediateContext->IASetInputLayout(m_layout.Get());

		m_batch->Begin();
		for (auto& item : debugDrawCommands)
		{
			switch (item.type)
			{
			case EDebugMeshDraw::Box:
				Draw(m_batch.get(), item.boundingBox, item.color);
				break;
			case EDebugMeshDraw::Sphere:
				Draw(m_batch.get(), item.boundingSphere, item.color);
				break;
			case EDebugMeshDraw::Ray:
				DrawRay(m_batch.get(), item.ray.position, item.ray.direction, false, item.color);
				break;
			case EDebugMeshDraw::Frustom:
				Draw(m_batch.get(), item.frustom, item.color);
				break;


			default:
				break;
			}

		}
		m_batch->End();
	}

	allDrawCommands.clear();
	postProcesShaders.clear();
	deferredDrawCommands.clear();
	forwardDrawCommands.clear();
	alphaDrawCommands.clear();
	allDrawCommandsOrigin.clear();
	postProcesCommands.clear();
	drawCommandBindable.clear();
	uiDrawCommands.clear();
	uiDrawCommands2.clear();
	textDrawCommands.clear();
	debugDrawCommands.clear();
	particleDrawCommands.clear();
	uiStencilMaskingCommands.clear();
	uiDrawingOnMaskingCommands.clear();
	render1uiStencilMaskingCommands.clear();
	render1uiDrawingOnMaskingCommands.clear();
	computeShaderSets.clear();
	dispatchs.clear();

	for (auto& item : bindables)
	{
		BindBinadble(item, true);
	}
}

void DefferdRenderer::SetCameraMatrix(const Matrix& world)
{
	cameraWorld = world;
}

void DefferdRenderer::SetPerspectiveProjection(float fov, float nearZ, float farZ)
{
	this->fov = fov;
	this->nearZ = nearZ;
	this->farZ = farZ;
	cameraProjection = DirectX::XMMatrixPerspectiveFovLH(fov, (float)width / (float)height, nearZ, farZ);
}

void DefferdRenderer::SetOrthographicProjection(float nearZ, float farZ)
{
	this->fov = 0;
	this->nearZ = nearZ;
	this->farZ = farZ;

	cameraProjection = DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ);
}


std::unique_ptr<DirectX::SpriteFont> DefferdRenderer::CreateSpriteFont(std::wstring_view path)
{
	return std::make_unique<DirectX::SpriteFont>(device.Get(), path.data());
}




void DefferdRenderer::ProcessDrawCommand(MeshDrawCommand& drawCommands)
{
	if (!drawCommands.meshData.vertexBuffer) return;
	ID3D11Buffer* vertexBuffer[1] = { drawCommands.meshData.vertexBuffer };
	UINT stride = drawCommands.meshData.vertexStride;
	UINT offset = 0;

	immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &stride, &offset);
	immediateContext->IASetIndexBuffer(drawCommands.meshData.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	immediateContext->IASetInputLayout(drawCommands.meshData.vertexShader);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto& i : drawCommands.meshData.shaderResources)
	{
		BindBinadble(i);
	}

	immediateContext->VSSetShader(drawCommands.meshData.vertexShader, nullptr, 0);
	immediateContext->PSSetShader(drawCommands.materialData.pixelShader, nullptr, 0);
	for (auto& i : drawCommands.materialData.shaderResources)
	{
		BindBinadble(i);
	}

	immediateContext->DrawIndexed(drawCommands.meshData.indexCounts, 0, 0);
}

void DefferdRenderer::ProcessDrawCommand(UIDrawCommand& drawCommands)
{
	ID3D11Buffer* vertexBuffer[1] = { uiVertexBuffer };
	UINT stride = uiVertexStride;
	UINT offset = 0;

	BindBinadble(Binadble
				 {
					 .shaderType = EShaderType::Vertex,
					 .bindableType = EShaderBindable::ConstantBuffer,
					 .slot = 0,
					 .bind = (ID3D11Buffer*)drawCommands.transformBuffer
				 });

	BindBinadble(Binadble
				 {
					 .shaderType = EShaderType::Pixel,
					 .bindableType = EShaderBindable::ConstantBuffer,
					 .slot = 0,
					 .bind = (ID3D11Buffer*)drawCommands.colorBuffer
				 });

	immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &stride, &offset);
	immediateContext->IASetIndexBuffer(uiIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	immediateContext->IASetInputLayout(uiVertexShader);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediateContext->VSSetShader(uiVertexShader, nullptr, 0);
	immediateContext->PSSetShader(uiPixelShader, nullptr, 0);

	ID3D11ShaderResourceView* srv[1] = { (ID3D11ShaderResourceView*)drawCommands.texture };
	immediateContext->PSSetShaderResources(0, std::size(srv), std::data(srv));

	immediateContext->DrawIndexed(uiIndexCounts, 0, 0);

}

void DefferdRenderer::ProcessDrawCommand(UIMaterialDrawCommand2& drawCommands)
{
	ID3D11Buffer* vertexBuffer[1] = { uiVertexBuffer };
	UINT stride = uiVertexStride;
	UINT offset = 0;

	BindBinadble(Binadble
				 {
					 .shaderType = EShaderType::Vertex,
					 .bindableType = EShaderBindable::ConstantBuffer,
					 .slot = 0,
					 .bind = (ID3D11Buffer*)drawCommands.transformBuffer
				 });
	for (size_t i = drawCommands.shaderResourcesStart; i < drawCommands.shaderResourcesEnd; i++)
	{
		BindBinadble(drawCommandBindable[i]);
	}

	immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &stride, &offset);
	immediateContext->IASetIndexBuffer(uiIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	immediateContext->IASetInputLayout(uiVertexShader);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediateContext->VSSetShader(uiVertexShader, nullptr, 0);
	immediateContext->PSSetShader(drawCommands.pixelShader, nullptr, 0);

	immediateContext->DrawIndexed(uiIndexCounts, 0, 0);

}

void DefferdRenderer::ProcessDrawCommand(TextDrawCommand& drawCommands)
{
	//spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, nullptr, m_states->PointClamp());
	spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_states->NonPremultiplied(), m_states->LinearClamp());
	drawCommands.sprite_font->DrawString(spriteBatch.get(), 
										 drawCommands.text.c_str(), 
										 DirectX::XMFLOAT2(drawCommands.x, drawCommands.y), 
										 drawCommands.color, 
										 0.0f, 
										 DirectX::XMFLOAT2(0, 0), 
										 drawCommands.size_mult);
	spriteBatch->End();
}

void DefferdRenderer::ProcessDrawCommand(PostProcesCommand2& drawCommands)
{
	if (drawCommands.isMipMap)
	{
		immediateContext->GenerateMips(deferredBuffer[0]);
	}
	for (size_t i = drawCommands.shaderResourcesStart; i < drawCommands.shaderResourcesEnd; i++)
	{
		BindBinadble(drawCommandBindable[i]);
	}

	for (size_t i = drawCommands.computeShaderStart; i < drawCommands.computeShaderEnd; i++)
	{
		if (computeShaderSets[i])
		{
			computeShaderSets[i]();
		}
		immediateContext->CSSetShader(postProcesShaders[i], nullptr, 0); 
		immediateContext->Dispatch(dispatchs[i].x / 64 + 1, dispatchs[i].y, dispatchs[i].z);
		//immediateContext->Dispatch(width / 64 + 1, height, 1);
	}

	for (size_t i = drawCommands.shaderResourcesStart; i < drawCommands.shaderResourcesEnd; i++)
	{
		BindBinadble(drawCommandBindable[i], true);
	}
}

void DefferdRenderer::ProcessDrawCommand(ParticleDrawCommand2& drawCommands)
{

	// CS 업데이트
	{
		ID3D11UnorderedAccessView* nullUAV[3]{ nullptr, nullptr};
		ID3D11ShaderResourceView* nullSRV[1]{ nullptr };
		ID3D11Buffer* nullCB[1]{ nullptr };

		ID3D11UnorderedAccessView* bindUAV[3];
		ID3D11ShaderResourceView* bindSRV[1];
		ID3D11Buffer* bindCB[1];

		UINT bindUAVCount[3]{ drawCommands.instanceCount, 0, 0 };
		bindUAV[0] = drawCommands.addParticleBuffer[0];
		bindUAV[1] = drawCommands.particleBuffer;
		bindUAV[2] = drawCommands.deadParticleBuffer;

		bindSRV[0] = drawCommands.addParticleBuffer[1];
		bindCB[0] = drawCommands.optionBuffer;

		immediateContext->CSSetShader(particleComputeShader, nullptr, 0);
		immediateContext->CSSetUnorderedAccessViews(0, std::size(bindUAV), bindUAV, bindUAVCount);
		immediateContext->CSSetShaderResources(0, std::size(bindSRV), bindSRV);
		immediateContext->CSSetConstantBuffers(4, std::size(bindCB), bindCB);

		immediateContext->Dispatch(drawCommands.addParticleCount / 64 + 1, 1, 1);

		immediateContext->CSSetUnorderedAccessViews(0, std::size(nullUAV), nullUAV, nullptr);
		immediateContext->CSSetShaderResources(0, std::size(nullSRV), nullSRV);
		immediateContext->CSSetConstantBuffers(0, std::size(nullCB), nullCB);
		
	}


	immediateContext->CopyStructureCount(drawCommands.instanceCountBuffer, 0, drawCommands.particleBuffer);

	drawCommands.deadParticleStagingBuffer.Copy(drawCommands.deadParticleBuffer.GetBuffer());
	drawCommands.instanceCountStagingBuffer.Copy(drawCommands.instanceCountBuffer);
	//Render Pass 출략
	{
		immediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		immediateContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
		immediateContext->IASetInputLayout(nullptr);
		immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		immediateContext->VSSetShader(particleVertexShader, nullptr, 0);
		immediateContext->GSSetShader(particleGeometryShader, nullptr, 0);
		immediateContext->PSSetShader(drawCommands.pixelShader, nullptr, 0);

		for (size_t i = drawCommands.psShaderResourcesStart; i < drawCommands.psShaderResourcesEnd; i++)
		{
			BindBinadble(drawCommandBindable[i]);
		}

		immediateContext->DrawInstancedIndirect(drawCommands.instanceCountBuffer, 0);

		for (size_t i = drawCommands.psShaderResourcesStart; i < drawCommands.psShaderResourcesEnd; i++)
		{
			BindBinadble(drawCommandBindable[i], true);
		}
	}


	immediateContext->VSSetShader(nullptr, nullptr, 0);
	immediateContext->GSSetShader(nullptr, nullptr, 0);
	immediateContext->PSSetShader(nullptr, nullptr, 0);
	


}

void DefferdRenderer::ProcessDrawCommands(std::vector<MeshDrawCommand2*>& drawCommands, bool isWithMaterial)
{
	for (auto& command : drawCommands)
	{
		ID3D11Buffer* vertexBuffer[1] = { command->vertexBuffer };
		UINT stride = command->vertexStride;
		UINT offset = 0;

		immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &stride, &offset);
		immediateContext->IASetIndexBuffer(command->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		immediateContext->IASetInputLayout(command->vertexShader);
		immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		for (size_t i = command->vsShaderResourcesStart; i < command->vsShaderResourcesEnd; i++)
		{
			BindBinadble(drawCommandBindable[i]);
		}

		immediateContext->VSSetShader(command->vertexShader, nullptr, 0);
		if (isWithMaterial)
		{
			immediateContext->PSSetShader(command->pixelShader, nullptr, 0);
			for (size_t i = command->psShaderResourcesStart; i < command->psShaderResourcesEnd; i++)
			{
				BindBinadble(drawCommandBindable[i]);
			}
		}

		immediateContext->DrawIndexed(command->indexCounts, 0, 0);
	}
}

struct BindHelper
{
	using BindConstantBufferFunction = std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11Buffer* const*)>;
	using BindShaderResourceFunction = std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11ShaderResourceView* const*)>;
	using BindUAVFunction = std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11UnorderedAccessView* const*, const UINT*)>;
	using BindSamplerFunction = std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11SamplerState* const*)>;

	static BindConstantBufferFunction ConstantBufferBindFunction[EShaderType::MAX];
	static BindShaderResourceFunction ShaderResourceBindFunction[EShaderType::MAX];
	static BindUAVFunction UAVBindFunction[EShaderType::MAX];
	static BindSamplerFunction SamplerBindFunction[EShaderType::MAX];

	template<EShaderBindable::Type shaderBindable>
	static auto Get()
	{
		if constexpr (shaderBindable == EShaderBindable::ConstantBuffer)
		{
			return ConstantBufferBindFunction;
		}
		else if constexpr (shaderBindable == EShaderBindable::ShaderResource)
		{
			return ShaderResourceBindFunction;
		}
		else if constexpr (shaderBindable == EShaderBindable::UnorderedAccess)
		{
			return UAVBindFunction;
		}
		else if constexpr (shaderBindable == EShaderBindable::Sampler)
		{
			return SamplerBindFunction;
		}
		else
		{
			//static_assert(false);
		}
	}
};

void DefferdRenderer::BindBinadble(const Binadble& bindable, bool unBind)
{
	if (!bindable.bind) return;
	switch (bindable.bindableType)
	{
	case EShaderBindable::ConstantBuffer:
	{
		ComPtr<ID3D11Buffer> buffer;
		bindable.bind.As(&buffer);

		ID3D11Buffer* buffers[1] = { !unBind ? buffer.Get() : nullptr };
		auto& function = BindHelper::Get<EShaderBindable::ConstantBuffer>()[bindable.shaderType];
		std::invoke(function, immediateContext.Get(), bindable.slot, std::size(buffers), buffers);
	}
	break;

	case EShaderBindable::ShaderResource:
	{
		ComPtr<ID3D11ShaderResourceView> srv;
		bindable.bind.As(&srv);

		ID3D11ShaderResourceView* resources[1] = { !unBind ? srv.Get() : nullptr };
		auto& function = BindHelper::Get<EShaderBindable::ShaderResource>()[bindable.shaderType];
		std::invoke(function, immediateContext.Get(), bindable.slot, std::size(resources), resources);
	}
	break;

	case EShaderBindable::Sampler:
	{
		ComPtr<ID3D11SamplerState> sampler;
		bindable.bind.As(&sampler);


		ID3D11SamplerState* resources[1] = { !unBind ? sampler.Get() : nullptr };
		auto& function = BindHelper::Get<EShaderBindable::Sampler>()[bindable.shaderType];
		std::invoke(function, immediateContext.Get(), bindable.slot, std::size(resources), resources);
	}
	break;
	case EShaderBindable::UnorderedAccess:
	{
		ComPtr<ID3D11UnorderedAccessView> uav;
		bindable.bind.As(&uav);

		ID3D11UnorderedAccessView* resources[1] = { !unBind ? uav.Get() : nullptr };
		auto& function = BindHelper::Get<EShaderBindable::UnorderedAccess>()[bindable.shaderType];
		std::invoke(function, immediateContext.Get(), bindable.slot, std::size(resources), resources, nullptr);
	}
	break;

	default:
		assert(false);
		break;
	}

}

void DefferdRenderer::ProcessDrawCommand(StencilMaskingCommand2& drawCommands)
{
	ID3D11Buffer* vertexBuffer[1] = { uiVertexBuffer };
	UINT stride = uiVertexStride;
	UINT offset = 0;

	BindBinadble(Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)drawCommands.transformBuffer
		});
	for (size_t i = drawCommands.shaderResourcesStart; i < drawCommands.shaderResourcesEnd; i++)
	{
		BindBinadble(drawCommandBindable[i]);
	}

	immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &stride, &offset);
	immediateContext->IASetIndexBuffer(uiIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	immediateContext->IASetInputLayout(uiVertexShader);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediateContext->VSSetShader(uiVertexShader, nullptr, 0);
	immediateContext->PSSetShader(drawCommands.pixelShader, nullptr, 0);

	immediateContext->DrawIndexed(uiIndexCounts, 0, 0);
}

void DefferdRenderer::ProcessDrawCommand(DrawingOnMaskingCommand2& drawCommands)
{
	ID3D11Buffer* vertexBuffer[1] = { uiVertexBuffer };
	UINT stride = uiVertexStride;
	UINT offset = 0;

	BindBinadble(Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)drawCommands.transformBuffer
		});
	for (size_t i = drawCommands.shaderResourcesStart; i < drawCommands.shaderResourcesEnd; i++)
	{
		BindBinadble(drawCommandBindable[i]);
	}

	immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &stride, &offset);
	immediateContext->IASetIndexBuffer(uiIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	immediateContext->IASetInputLayout(uiVertexShader);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediateContext->VSSetShader(uiVertexShader, nullptr, 0);
	immediateContext->PSSetShader(drawCommands.pixelShader, nullptr, 0);

	immediateContext->DrawIndexed(uiIndexCounts, 0, 0);
}

void DefferdRenderer::ProcessDrawCommand(UIRender1StencilMaskingCommand& drawCommands)
{
	ID3D11Buffer* vertexBuffer[1] = { uiVertexBuffer };
	UINT stride = uiVertexStride;
	UINT offset = 0;

	BindBinadble(Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)drawCommands.transformBuffer
		});

	BindBinadble(Binadble
		{
			.shaderType = EShaderType::Pixel,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)drawCommands.colorBuffer
		});

	immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &stride, &offset);
	immediateContext->IASetIndexBuffer(uiIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	immediateContext->IASetInputLayout(uiVertexShader);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediateContext->VSSetShader(uiVertexShader, nullptr, 0);
	immediateContext->PSSetShader(uiPixelShader, nullptr, 0);

	ID3D11ShaderResourceView* srv[1] = { (ID3D11ShaderResourceView*)drawCommands.texture };
	immediateContext->PSSetShaderResources(0, std::size(srv), std::data(srv));

	immediateContext->DrawIndexed(uiIndexCounts, 0, 0);
}

void DefferdRenderer::ProcessDrawCommand(UIRender1DrawingOnMaskingCommand& drawCommands)
{
	ID3D11Buffer* vertexBuffer[1] = { uiVertexBuffer };
	UINT stride = uiVertexStride;
	UINT offset = 0;

	BindBinadble(Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)drawCommands.transformBuffer
		});

	BindBinadble(Binadble
		{
			.shaderType = EShaderType::Pixel,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)drawCommands.colorBuffer
		});

	immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &stride, &offset);
	immediateContext->IASetIndexBuffer(uiIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	immediateContext->IASetInputLayout(uiVertexShader);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediateContext->VSSetShader(uiVertexShader, nullptr, 0);
	immediateContext->PSSetShader(uiPixelShader, nullptr, 0);

	ID3D11ShaderResourceView* srv[1] = { (ID3D11ShaderResourceView*)drawCommands.texture };
	immediateContext->PSSetShaderResources(0, std::size(srv), std::data(srv));

	immediateContext->DrawIndexed(uiIndexCounts, 0, 0);
}

BindHelper::BindConstantBufferFunction BindHelper::ConstantBufferBindFunction[EShaderType::MAX] =
{
	&ID3D11DeviceContext::VSSetConstantBuffers,
	&ID3D11DeviceContext::PSSetConstantBuffers,
	&ID3D11DeviceContext::GSSetConstantBuffers,
	&ID3D11DeviceContext::CSSetConstantBuffers,
	&ID3D11DeviceContext::HSSetConstantBuffers,
	&ID3D11DeviceContext::DSSetConstantBuffers
};

BindHelper::BindShaderResourceFunction BindHelper::ShaderResourceBindFunction[EShaderType::MAX] =
{
	&ID3D11DeviceContext::VSSetShaderResources,
	&ID3D11DeviceContext::PSSetShaderResources,
	&ID3D11DeviceContext::GSSetShaderResources,
	&ID3D11DeviceContext::CSSetShaderResources,
	&ID3D11DeviceContext::HSSetShaderResources,
	&ID3D11DeviceContext::DSSetShaderResources
};

BindHelper::BindUAVFunction BindHelper::UAVBindFunction[EShaderType::MAX] =
{
	nullptr,
	nullptr,
	nullptr,
	&ID3D11DeviceContext::CSSetUnorderedAccessViews,
	nullptr,
	nullptr
};

BindHelper::BindSamplerFunction BindHelper::SamplerBindFunction[EShaderType::MAX] =
{
	&ID3D11DeviceContext::VSSetSamplers,
	&ID3D11DeviceContext::PSSetSamplers,
	&ID3D11DeviceContext::GSSetSamplers,
	&ID3D11DeviceContext::CSSetSamplers,
	&ID3D11DeviceContext::HSSetSamplers,
	&ID3D11DeviceContext::DSSetSamplers
};