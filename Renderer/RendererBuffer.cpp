#include "RendererBuffer.h"
#include <d3d11.h>

RendererBuffer::RendererBuffer() = default;
RendererBuffer::~RendererBuffer() = default;

void RendererBuffer::Load(ComPtr<ID3D11Buffer> buffer)
{
	this->buffer = std::move(buffer);
	RendererUtility::GetDevice()->GetImmediateContext(&immediateContext);
}

void RendererBuffer::Init(D3D11_BUFFER_DESC bufferDesc, size_t bufferSize, _In_opt_ const void* data)
{
	HRESULT result;

	bufferDesc.ByteWidth = bufferSize;
	if (bufferDesc.Usage == D3D11_USAGE_DYNAMIC)
	{
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		isDynamic = true;
	}

	D3D11_SUBRESOURCE_DATA initData = {};
	D3D11_SUBRESOURCE_DATA* initDataPointer = nullptr;
	if (data)
	{
		initData.pSysMem = data;
		initDataPointer = &initData;
	}

	result = RendererUtility::GetDevice()->CreateBuffer(&bufferDesc, initDataPointer, &buffer);
	Check(result);

	RendererUtility::GetDevice()->GetImmediateContext(&immediateContext);
}

void RendererBuffer::Update(const void* data)
{
	if (!isDynamic)
	{
		immediateContext->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		auto result = immediateContext->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (result == S_OK)
		{
			memcpy(mappedResource.pData, data, mappedResource.RowPitch);
			immediateContext->Unmap(buffer.Get(), 0);
		}
	}
}

void RendererBuffer::Copy(const RendererBuffer& rhs)
{
	immediateContext->CopyResource(this->buffer.Get(), rhs.buffer.Get());
}

DynamicBufferModifier::DynamicBufferModifier()
{

}

DynamicBufferModifier::~DynamicBufferModifier()
{
	if (isMapped)
	{
		Unmap();
	}
}

void* DynamicBufferModifier::Map(RendererBuffer& buffer, size_t size, size_t offset, D3D11_MAP mapFlag)
{
	RendererUtility::GetDevice()->GetImmediateContext(&immediateContext);
	this->buffer = buffer;

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	HRESULT result = immediateContext->Map(buffer, 0, mapFlag, 0, &mappedResource);
	if (FAILED(result))
	{
		return nullptr;
	}
	isMapped = true;

	return mappedResource.pData;
}

void DynamicBufferModifier::Unmap()
{
	isMapped = false;
	immediateContext->Unmap(buffer.Get(), 0);
}
