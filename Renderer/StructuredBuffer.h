#pragma once

#include "RendererCore.h"
#include "ShaderResource.h"
#include "RendererBuffer.h"
#include <vector>
#include <typeinfo>
#include <d3d11.h>

struct StructuredBufferDESC 
{
	uint8_t isSRV : 1 { false };
	uint8_t isDynamic : 1 { false };


	uint8_t isUAV : 1 { false };
	uint8_t isAppend : 1 { false };
};;

template<typename Container>
concept ContiguousContainer = std::ranges::contiguous_range<Container>;

struct StructuredBuffer : public ShaderResource
{
public:
	StructuredBuffer();
	~StructuredBuffer();

public:
	template<ContiguousContainer Container>
	void Set(const Container& contaioner, StructuredBufferDESC desc = {});

	template<ContiguousContainer Container>
	void Init(const Container& contaioner, StructuredBufferDESC desc = {});

	template<ContiguousContainer Container>
	void Update(const Container& contaioner);

	operator ID3D11ShaderResourceView* () const { return srv.Get(); }
	operator ID3D11UnorderedAccessView* () const { return uav.Get(); }
	RendererBuffer& GetBuffer() { return buffer; }

	void CreateBuffer(uint32_t stride, int newSize, StructuredBufferDESC desc);
	void CreateView(int newSize);

	uint32_t GetCapicity() const { return capicity; }
private:
	RendererBuffer buffer{};
	ComPtr<ID3D11ShaderResourceView> srv{ nullptr };
	ComPtr<ID3D11UnorderedAccessView> uav{ nullptr };

	const std::type_info* typeInfo{ nullptr };
	uint32_t capicity{ 0 };
	uint32_t size{ 0 };
	uint8_t isInit : 1 { false }; 
	StructuredBufferDESC desc;
};

template<ContiguousContainer Container>
inline void StructuredBuffer::Set(const Container& contaioner, StructuredBufferDESC desc)
{
	using ValueType = Container::value_type;
	if (!isInit || (typeInfo  && *typeInfo != typeid(ValueType)))
	{
		Init(contaioner, desc);
		isInit = true;
	}
	else
	{
		Update(contaioner);
	}
}


template<ContiguousContainer Container>
inline void StructuredBuffer::Init(const Container& contaioner, StructuredBufferDESC desc)
{
	using ValueType = Container::value_type;
	CreateBuffer(sizeof(ValueType), contaioner.size(), desc);
	typeInfo = &typeid(ValueType);

	if (capicity)
	{
		CreateView(capicity);
	}
}

template<ContiguousContainer Container>
inline void StructuredBuffer::Update(const Container& contaioner)
{
	if (contaioner.size() > capicity)
	{
		Init(contaioner, desc);
	}
	if (contaioner.size() != size)
	{
		CreateView(contaioner.size());
	}
	if (buffer && contaioner.size() > 0)
	{
		buffer.Update(contaioner.data());
	}
}