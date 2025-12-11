#include "Texture.h"


Texture::Texture() = default;
Texture::~Texture() = default;

void Texture::CreateTexture(std::unique_ptr<DirectX::ScratchImage>&& image, 
							ETextureUsage::Type textureUsage,
							D3D11_SHADER_RESOURCE_VIEW_DESC* srvFormat,
							D3D11_UNORDERED_ACCESS_VIEW_DESC* uavFormat)
{
	HRESULT result;
	ID3D11Device* device = RendererUtility::GetDevice();
	ComPtr<ID3D11Resource> textureResource;

	result = DirectX::CreateTextureEx(device,
									  image->GetImages(),
									  image->GetImageCount(),
									  image->GetMetadata(),
									  D3D11_USAGE_DEFAULT,
									  textureUsage,
									  0,
									  0,
									  DirectX::CREATETEX_DEFAULT,
									  &textureResource);
	Check(result);

	LoadTexture(textureResource.Get(), textureUsage, srvFormat, uavFormat);
}

void Texture::LoadTexture(ID3D11Resource* resource,
						  ETextureUsage::Type textureUsage, 
						  _In_opt_ D3D11_SHADER_RESOURCE_VIEW_DESC* srvFormat,
						  _In_opt_ D3D11_UNORDERED_ACCESS_VIEW_DESC* uavFormat)
{
	HRESULT result;
	ID3D11Device* device = RendererUtility::GetDevice();

	texture = resource;
	if (textureUsage & ETextureUsage::SRV)
	{
		result = device->CreateShaderResourceView(texture.Get(), srvFormat, &shaderResourceView);
		Check(result);
	}
	if (textureUsage & ETextureUsage::RTV)
	{
		result = (device->CreateRenderTargetView(texture.Get(), nullptr, &renderTargetView));
		Check(result);
	}
	if (textureUsage & ETextureUsage::DSV)
	{
		CD3D11_DEPTH_STENCIL_VIEW_DESC desc
		{
			D3D11_DSV_DIMENSION_TEXTURE2D,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		};
		result = device->CreateDepthStencilView(texture.Get(), &desc, &depthStencilView);
		Check(result);
	}
	if (textureUsage & ETextureUsage::UAV)
	{
		result = device->CreateUnorderedAccessView(texture.Get(), uavFormat, &unorderedAccessView);
		Check(result);
	}
}

void Texture::LoadTexture(ID3D11ShaderResourceView* srv)
{
	shaderResourceView = srv;
}

Texture::operator ID3D11Texture2D* () const
{
	ID3D11Texture2D* texture2D = nullptr;
	texture->QueryInterface(&texture2D);
	texture2D->Release();
	return texture2D;
}


void Texture::ClearTexture(ETextureUsage::Type type, float* color)
{
	ID3D11Device* device = RendererUtility::GetDevice();
	ComPtr<ID3D11DeviceContext> immediateContext;
	device->GetImmediateContext(&immediateContext);

	switch (type)
	{
	case ETextureUsage::RTV:
		immediateContext->ClearRenderTargetView(renderTargetView.Get(), color);
		break;
	case ETextureUsage::DSV:
		immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		break;
	case ETextureUsage::UAV:
		immediateContext->ClearUnorderedAccessViewFloat(unorderedAccessView.Get(), color);
		break;
	default:
		break;
	}



}