#pragma once
#include "RendererCore.h"
#include "ShaderResource.h"
#include <span>

struct RendererBuffer : public ShaderResource
{
public:
	RendererBuffer();
	~RendererBuffer();

public:
	void Load(ComPtr<ID3D11Buffer> buffer);
	void Init(D3D11_BUFFER_DESC bufferDesc, size_t bufferSize, _In_opt_ const void* data = nullptr);
	void Update(const void* data);

	void Copy(const RendererBuffer& buffer);
public:
	operator ID3D11Buffer*() { return buffer.Get(); }

private:
	ComPtr<ID3D11Buffer> buffer{};
	ComPtr<ID3D11DeviceContext> immediateContext{};
	uint8_t isDynamic : 1{ false };
};

#include <d3d11.h>



struct DynamicBufferModifier
{
public:
	DynamicBufferModifier();
	~DynamicBufferModifier();

public:
	void* Map(RendererBuffer& buffer, size_t size, size_t offset = 0, D3D11_MAP mapFlag = D3D11_MAP_WRITE_DISCARD);
	void Unmap();

	template<class Type>
	Type* Map(RendererBuffer& buffer, size_t size, size_t offset = 0, D3D11_MAP mapFlag = D3D11_MAP_WRITE_DISCARD)
	{
		return static_cast<Type*>(Map(buffer, size, offset, mapFlag));
	}
private:
	uint8_t isMapped : 1{ false };
	ComPtr<ID3D11DeviceContext> immediateContext{};
	ComPtr<ID3D11Buffer> buffer{};

};

