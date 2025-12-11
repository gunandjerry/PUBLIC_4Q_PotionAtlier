#include "StructuredBuffer.h"



StructuredBuffer::StructuredBuffer() = default;
StructuredBuffer::~StructuredBuffer() = default;



void StructuredBuffer::CreateBuffer(uint32_t stride, int newSize, StructuredBufferDESC desc)
{
	void* oldDataPTR = nullptr;
	if (buffer)
	{
		oldDataPTR = new char[newSize * stride];

		ComPtr<ID3D11DeviceContext> immediateContext{};
		RendererUtility::GetDevice()->GetImmediateContext(&immediateContext);

		D3D11_BUFFER_DESC StagingbufferDesc
		{
			.Usage = D3D11_USAGE_STAGING,
			.CPUAccessFlags = D3D11_CPU_ACCESS_READ,
			.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
			.StructureByteStride = stride
		};
		RendererBuffer stagingBuffer;
		stagingBuffer.Init(StagingbufferDesc, capicity * stride);
		immediateContext->CopyResource(stagingBuffer, buffer);

		D3D11_MAPPED_SUBRESOURCE oldData;
		immediateContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &oldData);
		memcpy(oldDataPTR, oldData.pData, capicity * stride);
		immediateContext->Unmap(stagingBuffer, 0);
	}

	int bufferSize = stride * newSize;
	D3D11_BUFFER_DESC bufferDesc
	{
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		.StructureByteStride = stride
	};

	bufferDesc.BindFlags |= desc.isUAV ? D3D11_BIND_UNORDERED_ACCESS : 0;
	bufferDesc.Usage = desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	if (bufferSize)
	{
		buffer.Init(bufferDesc, bufferSize, oldDataPTR);
	}


	capicity = newSize;
	isInit = true;

	this->desc = desc;
	if (oldDataPTR) delete[] oldDataPTR;
}

void StructuredBuffer::CreateView(int newSize)
{
	HRESULT result;
	if (desc.isSRV)
	{
		if (newSize > 0)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = newSize;

			result = RendererUtility::GetDevice()->CreateShaderResourceView((ID3D11Buffer*)buffer, desc.isAppend ? nullptr : &srvDesc, &srv);
			Check(result);
		}
		else if(desc.isAppend)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = capicity;

			result = RendererUtility::GetDevice()->CreateShaderResourceView((ID3D11Buffer*)buffer, desc.isAppend ? nullptr : &srvDesc, &srv);
			Check(result);
		}
		else
		{
			srv.Reset();
		}
	}

	if (desc.isUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = desc.isAppend ? capicity : 0;
		uavDesc.Buffer.Flags = desc.isAppend ? D3D11_BUFFER_UAV_FLAG_APPEND : 0;
		result = RendererUtility::GetDevice()->CreateUnorderedAccessView((ID3D11Buffer*)buffer, &uavDesc, &uav);
		Check(result);
	}

	this->size = newSize;
}