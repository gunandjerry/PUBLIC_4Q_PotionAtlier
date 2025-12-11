#include "PostProcessUtility.h"
#include <Manager\HLSLManager.h>
#include <Utility/SerializedUtility.h>
#include <framework.h>


EdgeDetection::EdgeDetection()
{
	Texture testTexture[2];


	postProcesCommand.computeShader.resize(4);
	postProcesCommand.computeShaderSet.resize(4);
	postProcesCommand.dispatchDatas.resize(4);
	drawSpeed = 0;

	ComPtr<ID3D11ComputeShader> shader;
	hlslManager.CreateSharingShader(L"Resource/Shader/EdgeDetection.hlsl", &shader);
	postProcesCommand.computeShader[0].LoadShader(shader.Get());

	hlslManager.CreateSharingShader(L"Resource/Shader/EdgeBlurVertical.hlsl", &shader);
	postProcesCommand.computeShader[1].LoadShader(shader.Get());

	hlslManager.CreateSharingShader(L"Resource/Shader/EdgeBlurHorizontal.hlsl", &shader);
	postProcesCommand.computeShader[2].LoadShader(shader.Get());

	hlslManager.CreateSharingShader(L"Resource/Shader/AddOriginEdge.hlsl", &shader);
	postProcesCommand.computeShader[3].LoadShader(shader.Get());



	std::unique_ptr<DirectX::ScratchImage> image;
	image = std::make_unique<DirectX::ScratchImage>();
	image->Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1920, 1080, 1, 1);
	testTexture[0].CreateTexture(std::move(image), ETextureUsage::SRV | ETextureUsage::UAV, nullptr, nullptr);

	image = std::make_unique<DirectX::ScratchImage>();
	image->Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1920, 1080, 1, 1);
	testTexture[1].CreateTexture(std::move(image), ETextureUsage::SRV | ETextureUsage::UAV, nullptr, nullptr);

	EdgeDetectionData tempValue{};

	cnstantBuffer.Set(tempValue);

	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::UnorderedAccess,
			.slot = 2,
			.bind = (ID3D11UnorderedAccessView*)testTexture[0]
		});
	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::UnorderedAccess,
			.slot = 3,
			.bind = (ID3D11UnorderedAccessView*)testTexture[1]
		});
	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)cnstantBuffer
		});
}

EdgeDetection::~EdgeDetection()
{
	hlslManager.ClearSharingShader(L"Resource/Shader/EdgeDetection.hlsl");
	hlslManager.ClearSharingShader(L"Resource/Shader/EdgeBlurVertical.hlsl");
	hlslManager.ClearSharingShader(L"Resource/Shader/EdgeBlurHorizontal.hlsl");
	hlslManager.ClearSharingShader(L"Resource/Shader/AddOriginEdge.hlsl");
}

void EdgeDetection::InspectorImguiDraw()
{
	PostProcessData::InspectorImguiDraw();
	if (ImGui::DragFloat("Edge Width", &value.width))
	{
		cnstantBuffer.Set(value);
	}
	if (ImGui::DragFloat("Edge Threadhold", &value.threadhold))
	{
		cnstantBuffer.Set(value);
	}
}

void EdgeDetection::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, value.width);
	Binary::Write::data(ofs, value.threadhold);
}

void EdgeDetection::Deserialized(std::ifstream& ifs)
{
	value.width = Binary::Read::data<float>(ifs);
	value.threadhold = Binary::Read::data<float>(ifs);

	cnstantBuffer.Set(value);
}

BlendColorGrading::BlendColorGrading()
{
	postProcesCommand.computeShader.resize(1);
	postProcesCommand.computeShaderSet.resize(1);
	postProcesCommand.dispatchDatas.resize(1);
	drawSpeed = 0;
	ComPtr<ID3D11ComputeShader> shader;
	hlslManager.CreateSharingShader(L"Resource/Shader/BlendColorGrading.hlsl", &shader);
	postProcesCommand.computeShader[0].LoadShader(shader.Get());

	BlendColorData tempValue{};
	cnstantBuffer.Set(tempValue);
	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)cnstantBuffer
		});

	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::ShaderResource,
			.slot = 10,
			.bind = nullptr
		});

	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::ShaderResource,
			.slot = 11,
			.bind = nullptr
		});



}

BlendColorGrading::~BlendColorGrading()
{
	hlslManager.ClearSharingShader(L"Resource/Shader/BlendColorGrading.hlsl");
}

void BlendColorGrading::InspectorImguiDraw()
{
	PostProcessData::InspectorImguiDraw();
	cnstantBuffer.Set(value);

	if (ImGui::DragFloat("Weight", &value.weight, 0.01f, 0.0f, 1.0f))
	{
		cnstantBuffer.Set(value);
	}
	if (ImGui::Button("Load Texture1"))
	{
		SetTexture(WinUtility::GetOpenFilePath(), 1);
	}
	if (ImGui::Button("Load Texture2"))
	{
		SetTexture(WinUtility::GetOpenFilePath(), 2);
	}
}

void BlendColorGrading::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, value.weight);
	Binary::Write::wstring(ofs, texturePath[0]);
	Binary::Write::wstring(ofs, texturePath[1]);
}

void BlendColorGrading::Deserialized(std::ifstream& ifs)
{
	value.weight = Binary::Read::data<float>(ifs);
	texturePath[0] = Binary::Read::wstring(ifs);
	texturePath[1] = Binary::Read::wstring(ifs);

	SetTexture(texturePath[0], 1);
	SetTexture(texturePath[1], 2);

	cnstantBuffer.Set(value);
}

void BlendColorGrading::SetTexture(std::wstring path, int index)
{
	if (path.empty()) return;
	if (index > 2) return;

	SetTexture(ColorGradingTexture::Get(path), index);
	texturePath[index - 1] = path;
}

void BlendColorGrading::SetTexture(const Texture& texture, int index)
{
	if (index > 2) return;
	postProcesCommand.shaderResources[index].bind = (ID3D11ShaderResourceView*)texture;
}

void BlendColorGrading::SetWeight(float weight)
{
	value.weight = weight;
	cnstantBuffer.Set(value);
}

EdgeDetection2::EdgeDetection2()
{
	Texture testTexture[2];


	postProcesCommand.computeShader.resize(4);
	postProcesCommand.computeShaderSet.resize(4);
	postProcesCommand.dispatchDatas.resize(4);
	drawSpeed = 0;

	ComPtr<ID3D11ComputeShader> shader;
	hlslManager.CreateSharingShader(L"Resource/Shader/EdgeDetection2.hlsl", &shader);
	postProcesCommand.computeShader[0].LoadShader(shader.Get());

	hlslManager.CreateSharingShader(L"Resource/Shader/EdgeBlurVertical.hlsl", &shader);
	postProcesCommand.computeShader[1].LoadShader(shader.Get());

	hlslManager.CreateSharingShader(L"Resource/Shader/EdgeBlurHorizontal.hlsl", &shader);
	postProcesCommand.computeShader[2].LoadShader(shader.Get());

	hlslManager.CreateSharingShader(L"Resource/Shader/AddOriginEdge.hlsl", &shader);
	postProcesCommand.computeShader[3].LoadShader(shader.Get());



	std::unique_ptr<DirectX::ScratchImage> image;
	image = std::make_unique<DirectX::ScratchImage>();
	image->Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1920, 1080, 1, 1);
	testTexture[0].CreateTexture(std::move(image), ETextureUsage::SRV | ETextureUsage::UAV, nullptr, nullptr);

	image = std::make_unique<DirectX::ScratchImage>();
	image->Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1920, 1080, 1, 1);
	testTexture[1].CreateTexture(std::move(image), ETextureUsage::SRV | ETextureUsage::UAV, nullptr, nullptr);

	EdgeDetection2Data tempValue{};

	cnstantBuffer.Set(tempValue);

	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::UnorderedAccess,
			.slot = 2,
			.bind = (ID3D11UnorderedAccessView*)testTexture[0]
		});
	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::UnorderedAccess,
			.slot = 3,
			.bind = (ID3D11UnorderedAccessView*)testTexture[1]
		});
	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)cnstantBuffer
		});

}

EdgeDetection2::~EdgeDetection2()
{
	hlslManager.ClearSharingShader(L"Resource/Shader/EdgeDetection2.hlsl");
	hlslManager.ClearSharingShader(L"Resource/Shader/EdgeBlurVertical.hlsl");
	hlslManager.ClearSharingShader(L"Resource/Shader/EdgeBlurHorizontal.hlsl");
	hlslManager.ClearSharingShader(L"Resource/Shader/AddOriginEdge.hlsl");
}

void EdgeDetection2::InspectorImguiDraw()
{
	PostProcessData::InspectorImguiDraw();
	bool isChange = false;
	isChange |= ImGui::ColorEdit4("Color", &value.color.x);
	isChange |= ImGui::DragFloat("Edge Width", &value.width, 0.1f);
	isChange |= ImGui::DragFloat("DethThreadhold", &value.dethThreadhold, 0.1f);
	isChange |= ImGui::DragFloat("NormalThreadhold", &value.normalThreadhold, 0.1f);

	if (isChange)
	{
		cnstantBuffer.Set(value);
	}

}

void EdgeDetection2::Serialized(std::ofstream& ofs)
{
	Binary::Write::Vector4(ofs, value.color);
	Binary::Write::data(ofs, value.width);
	Binary::Write::data(ofs, value.dethThreadhold);
	Binary::Write::data(ofs, value.normalThreadhold);
}

void EdgeDetection2::Deserialized(std::ifstream& ifs)
{
	value.color = Binary::Read::Vector4(ifs);
	value.width = Binary::Read::data<float>(ifs);
	value.dethThreadhold = Binary::Read::data<float>(ifs);
	value.normalThreadhold = Binary::Read::data<float>(ifs);

	cnstantBuffer.Set(value);
}

FXAA::FXAA()
{
	postProcesCommand.computeShader.resize(1);
	postProcesCommand.computeShaderSet.resize(1);
	postProcesCommand.dispatchDatas.resize(1);
	drawSpeed = 0;
	
	ComPtr<ID3D11ComputeShader> shader;
	hlslManager.CreateSharingShader(L"Resource/EngineShader/FXAA.hlsl", &shader);
	postProcesCommand.computeShader[0].LoadShader(shader.Get());
	
	value = {};
	value.screenWidth = D3D11_GameApp::GetClientSize().cx;
	value.screenHeight = D3D11_GameApp::GetClientSize().cy; 
	value.invScreenSize = { 1.0f / value.screenWidth, 1.0f / value.screenHeight };

	value.FXAA_SPAN_MAX = 8.0f;
	value.FXAA_REDUCE_MUL = 1.0f / 8.0f;
	value.FXAA_REDUCE_MIN = 1.0f / 128.0f;

	cnstantBuffer.Set(value);

	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)cnstantBuffer
		});


}

FXAA::~FXAA()
{
	hlslManager.ClearSharingShader(L"Resource/EngineShader/FXAA.hlsl");
}

void FXAA::InspectorImguiDraw()
{
	PostProcessData::InspectorImguiDraw();
	bool isChange = false;
	isChange |= ImGui::DragFloat("FXAA_SPAN_MAX", &value.FXAA_SPAN_MAX);
	isChange |= ImGui::DragFloat("FXAA_REDUCE_MUL", &value.FXAA_REDUCE_MUL);
	isChange |= ImGui::DragFloat("FXAA_REDUCE_MIN", &value.FXAA_REDUCE_MIN);


	if (isChange)
	{
		cnstantBuffer.Set(value);
	}
}

void FXAA::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, value.FXAA_SPAN_MAX);
	Binary::Write::data(ofs, value.FXAA_REDUCE_MUL);
	Binary::Write::data(ofs, value.FXAA_REDUCE_MIN);
}

void FXAA::Deserialized(std::ifstream& ifs)
{
	value.FXAA_SPAN_MAX = Binary::Read::data<float>(ifs);
	value.FXAA_REDUCE_MUL = Binary::Read::data<float>(ifs);
	value.FXAA_REDUCE_MIN = Binary::Read::data<float>(ifs);
	cnstantBuffer.Set(value);
}

Bloom::Bloom()
{
	postProcesCommand.computeShader.resize(13);
	postProcesCommand.computeShaderSet.resize(13);
	postProcesCommand.dispatchDatas.resize(13);
	drawSpeed = 0;
	ComPtr<ID3D11ComputeShader> shader;
	int shaderCount = 0;
	postProcesCommand.dispatchDatas[0] = { 1920 / 4, 1080 / 4, 1 };
	hlslManager.CreateSharingShader(L"Resource/EngineShader/BloomCurve.hlsl", &shader);

	postProcesCommand.dispatchDatas[1] = { postProcesCommand.dispatchDatas[0].x / 6, postProcesCommand.dispatchDatas[0].y / 6, 1 };
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());
	hlslManager.CreateSharingShader(L"Resource/EngineShader/DownSample6x6.hlsl", &shader);

	postProcesCommand.dispatchDatas[2] = { postProcesCommand.dispatchDatas[1].x / 6, postProcesCommand.dispatchDatas[1].y / 6, 1 };
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());
	hlslManager.CreateSharingShader(L"Resource/EngineShader/DownSample6x6.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());

	int blurStartCount = shaderCount ;

	hlslManager.CreateSharingShader(L"Resource/EngineShader/AddTexture.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());
	hlslManager.CreateSharingShader(L"Resource/EngineShader/BlurVertical5x5.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());
	hlslManager.CreateSharingShader(L"Resource/EngineShader/BlurHorizontal5x5.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());

	hlslManager.CreateSharingShader(L"Resource/EngineShader/AddTexture.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());
	hlslManager.CreateSharingShader(L"Resource/EngineShader/BlurVertical5x5.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());
	hlslManager.CreateSharingShader(L"Resource/EngineShader/BlurHorizontal5x5.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());

	hlslManager.CreateSharingShader(L"Resource/EngineShader/AddTexture.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());
	hlslManager.CreateSharingShader(L"Resource/EngineShader/BlurVertical5x5.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());
	hlslManager.CreateSharingShader(L"Resource/EngineShader/BlurHorizontal5x5.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());


	hlslManager.CreateSharingShader(L"Resource/EngineShader/BloomAdd.hlsl", &shader);
	postProcesCommand.computeShader[shaderCount++].LoadShader(shader.Get());

	value = {};
	value.curveThreshold = 1.0f;
	value.bloomIntensity = 1.0f;

	cnstantBuffer.Set(value);
	postProcesCommand.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Compute,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)cnstantBuffer
		});

	HRESULT result;
	ComPtr<ID3D11Texture2D> texture2D;
	ETextureUsage::Type usage = ETextureUsage::RTV | ETextureUsage::UAV | ETextureUsage::SRV;
	CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		1920,
		1080,
		1,
		1,
		usage
	};
	ID3D11Device* device = RendererUtility::GetDevice();
	ComPtr<ID3D11DeviceContext> immediateContext;
	device->GetImmediateContext(immediateContext.GetAddressOf());


	result = device->CreateTexture2D(&desc, nullptr, &texture2D);
	Check(result);
	blurTexture.LoadTexture(texture2D.Get(), usage);

	result = device->CreateTexture2D(&desc, nullptr, &texture2D);
	Check(result);
	tempTexture.LoadTexture(texture2D.Get(), usage);


	desc.Width = desc.Width / 4;
	desc.Height = desc.Height / 4;
	result = device->CreateTexture2D(&desc, nullptr, &texture2D);
	Check(result);
	textureMip[2].LoadTexture(texture2D.Get(), usage);

	desc.Width = desc.Width / 6;
	desc.Height = desc.Height / 6;
	result = device->CreateTexture2D(&desc, nullptr, &texture2D);
	Check(result);
	textureMip[1].LoadTexture(texture2D.Get(), usage);

	desc.Width = desc.Width / 6;
	desc.Height = desc.Height / 6;
	result = device->CreateTexture2D(&desc, nullptr, &texture2D);
	Check(result);
	textureMip[0].LoadTexture(texture2D.Get(), usage);



	postProcesCommand.computeShaderSet[0] =
		[this]()
		{
			float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			tempTexture.ClearTexture(ETextureUsage::UAV, color);
			ID3D11Device* device = RendererUtility::GetDevice();
			ComPtr<ID3D11DeviceContext> immediateContext;
			device->GetImmediateContext(immediateContext.GetAddressOf());

			ID3D11ShaderResourceView* nullsrv[1] = { nullptr };
			immediateContext->CSSetShaderResources(10, 1, nullsrv);
			ID3D11UnorderedAccessView* textures[1] = { (ID3D11UnorderedAccessView*)textureMip[2] };
			immediateContext->CSSetUnorderedAccessViews(1, std::size(textures), textures, nullptr);
		};

	postProcesCommand.computeShaderSet[1] =
		[this]()
		{
			ID3D11Device* device = RendererUtility::GetDevice();
			ComPtr<ID3D11DeviceContext> immediateContext;
			device->GetImmediateContext(immediateContext.GetAddressOf());

			ID3D11UnorderedAccessView* textures[1] = { (ID3D11UnorderedAccessView*)textureMip[1] };
			immediateContext->CSSetUnorderedAccessViews(1, std::size(textures), textures, nullptr);

			ID3D11ShaderResourceView* srv[1] = { textureMip[2] };
			immediateContext->CSSetShaderResources(10, 1, srv);
		};

	postProcesCommand.computeShaderSet[2] =
		[this]()
		{
			ID3D11Device* device = RendererUtility::GetDevice();
			ComPtr<ID3D11DeviceContext> immediateContext;
			device->GetImmediateContext(immediateContext.GetAddressOf());

			ID3D11UnorderedAccessView* textures[1] = { (ID3D11UnorderedAccessView*)textureMip[0] };
			immediateContext->CSSetUnorderedAccessViews(1, std::size(textures), textures, nullptr);

			ID3D11ShaderResourceView* srv[1] = { textureMip[1] };
			immediateContext->CSSetShaderResources(10, 1, srv);
		};

	constexpr int blurCount = 3;
	for (size_t i = 0; i < blurCount; i++)
	{
		postProcesCommand.computeShaderSet[blurStartCount + 0 + i * 3] =
			[this, mipsrv = textureMip[i]]()
			{
				ID3D11Device* device = RendererUtility::GetDevice();
				ComPtr<ID3D11DeviceContext> immediateContext;
				device->GetImmediateContext(immediateContext.GetAddressOf());

				ID3D11UnorderedAccessView* nulluav[1]{ nullptr };
				immediateContext->CSSetUnorderedAccessViews(1, std::size(nulluav), nulluav, nullptr);

				ID3D11ShaderResourceView* srv[1] = { mipsrv };
				immediateContext->CSSetShaderResources(10, std::size(srv), srv);

				ID3D11UnorderedAccessView* textures[1] = { (ID3D11UnorderedAccessView*)tempTexture };
				immediateContext->CSSetUnorderedAccessViews(1, std::size(textures), textures, nullptr);
			};

		postProcesCommand.computeShaderSet[blurStartCount + 1 + i * 3] =
			[this, mipSrv = textureMip[i]]()
			{
				ID3D11Device* device = RendererUtility::GetDevice();
				ComPtr<ID3D11DeviceContext> immediateContext;
				device->GetImmediateContext(immediateContext.GetAddressOf());
				ID3D11UnorderedAccessView* nulluav[1]{ nullptr };
				immediateContext->CSSetUnorderedAccessViews(1, std::size(nulluav), nulluav, nullptr);

				ID3D11ShaderResourceView* srv[1] = { (ID3D11ShaderResourceView*)tempTexture };
				immediateContext->CSSetShaderResources(10, std::size(srv), srv);

				ID3D11ShaderResourceView* srv2[1] = { (ID3D11ShaderResourceView*)mipSrv };
				immediateContext->CSSetShaderResources(11, 1, srv2);

				ID3D11UnorderedAccessView* textures[1] = { (ID3D11UnorderedAccessView*)blurTexture };
				immediateContext->CSSetUnorderedAccessViews(1, std::size(textures), textures, nullptr);
			};

		postProcesCommand.computeShaderSet[blurStartCount + 2 + i * 3] =
			[this]()
			{
				ID3D11Device* device = RendererUtility::GetDevice();
				ComPtr<ID3D11DeviceContext> immediateContext;
				device->GetImmediateContext(immediateContext.GetAddressOf());
				ID3D11UnorderedAccessView* nulluav[1]{ nullptr };
				immediateContext->CSSetUnorderedAccessViews(1, std::size(nulluav), nulluav, nullptr);

				ID3D11ShaderResourceView* srv[1] = { (ID3D11ShaderResourceView*)blurTexture };
				immediateContext->CSSetShaderResources(10, 1, srv);

				ID3D11UnorderedAccessView* textures[1] = { (ID3D11UnorderedAccessView*)tempTexture };
				immediateContext->CSSetUnorderedAccessViews(1, std::size(textures), textures, nullptr);
			};


	}

	postProcesCommand.computeShaderSet[blurStartCount + 2 + 2 * 3 + 1] =
		[this]()
		{
			ID3D11Device* device = RendererUtility::GetDevice();
			ComPtr<ID3D11DeviceContext> immediateContext;
			device->GetImmediateContext(immediateContext.GetAddressOf());
			ID3D11UnorderedAccessView* nulluav[1]{ nullptr };
			immediateContext->CSSetUnorderedAccessViews(1, std::size(nulluav), nulluav, nullptr);

			ID3D11ShaderResourceView* srv[2] = { (ID3D11ShaderResourceView*)tempTexture, nullptr };
			immediateContext->CSSetShaderResources(10, std::size(srv), srv);
		};
}

Bloom::~Bloom()
{
	hlslManager.ClearSharingShader(L"Resource/EngineShader/BloomCurve.hlsl");
	hlslManager.ClearSharingShader(L"Resource/EngineShader/DownSample6x6.hlsl");
	hlslManager.ClearSharingShader(L"Resource/EngineShader/DownSample6x6.hlsl");
	
	hlslManager.ClearSharingShader(L"Resource/EngineShader/AddTexture.hlsl");
	hlslManager.ClearSharingShader(L"Resource/EngineShader/BlurVertical5x5.hlsl");
	hlslManager.ClearSharingShader(L"Resource/EngineShader/BlurHorizontal5x5.hlsl");

	hlslManager.ClearSharingShader(L"Resource/EngineShader/AddTexture.hlsl");
	hlslManager.ClearSharingShader(L"Resource/EngineShader/BlurVertical5x5.hlsl");
	hlslManager.ClearSharingShader(L"Resource/EngineShader/BlurHorizontal5x5.hlsl");

	hlslManager.ClearSharingShader(L"Resource/EngineShader/AddTexture.hlsl");
	hlslManager.ClearSharingShader(L"Resource/EngineShader/BlurVertical5x5.hlsl");
	hlslManager.ClearSharingShader(L"Resource/EngineShader/BlurHorizontal5x5.hlsl");

	
	hlslManager.ClearSharingShader(L"Resource/EngineShader/BloomAdd.hlsl");


}

void Bloom::InspectorImguiDraw()
{
	PostProcessData::InspectorImguiDraw();
	bool isChange = false;
	isChange |= ImGui::Combo("Bloom Type", (int*)&value.bloomCurveMethod, "curve1\0curve2\0curve3\0");
	isChange |= ImGui::DragFloat("Curve Threshold", &value.curveThreshold, 0.1f);
	isChange |= ImGui::DragFloat("Bloom Intensity", &value.bloomIntensity, 0.1f);
	if (isChange)
	{
		cnstantBuffer.Set(value);
	}
}

void Bloom::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, value.bloomCurveMethod);
	Binary::Write::data(ofs, value.curveThreshold);
	Binary::Write::data(ofs, value.bloomIntensity);
}

void Bloom::Deserialized(std::ifstream& ifs)
{
	value.bloomCurveMethod = Binary::Read::data<int>(ifs);
	value.curveThreshold = Binary::Read::data<float>(ifs);
	value.bloomIntensity = Binary::Read::data<float>(ifs);
	cnstantBuffer.Set(value);
}

