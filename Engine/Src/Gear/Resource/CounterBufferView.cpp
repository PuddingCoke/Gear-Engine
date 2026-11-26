#include <Gear/Resource/CounterBufferView.h>

#include <Gear/Core/FMT.h>

namespace Gear::Resource
{
	CounterBufferView::CounterBufferView() :
		srvIndex(UINT32_MAX),
		uavIndex(UINT32_MAX),
		viewGPUHandle{},
		viewCPUHandle{},
		buffer(makeUnique<D3D12Resource::Buffer>(sizeof(uint32_t),
	                                             true,
	                                             D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
	{
		D3D12Core::DescriptorHandle* const descriptorHandle = createResourceDescriptorHandle(2);

		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Format = FMT::R32TL;
			desc.Buffer.FirstElement = 0;
			desc.Buffer.NumElements = 1;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

			GraphicsDevice::get()->CreateShaderResourceView(
				buffer->getResource(), &desc, descriptorHandle->getCurrentCPUHandle());

			srvIndex = descriptorHandle->getCurrentIndex();

			descriptorHandle->move();
		}

		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			desc.Format = FMT::R32TL;
			desc.Buffer.FirstElement = 0;
			desc.Buffer.NumElements = 1;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

			GraphicsDevice::get()->CreateUnorderedAccessView(
				buffer->getResource(), nullptr, &desc, descriptorHandle->getCurrentCPUHandle());

			uavIndex = descriptorHandle->getCurrentIndex();

			viewGPUHandle = descriptorHandle->getCurrentGPUHandle();

			D3D12Core::DescriptorHandle* const nonShaderVisibleHandle = createNonShaderVisibleResourceDescriptorHandle(1);

			GraphicsDevice::get()->CreateUnorderedAccessView(
				buffer->getResource(), nullptr, &desc, nonShaderVisibleHandle->getCurrentCPUHandle());

			viewCPUHandle = nonShaderVisibleHandle->getCurrentCPUHandle();
		}
	}

	CounterBufferView::CounterBufferView(const CounterBufferView& cbv) :
		ResourceBase(cbv),
		srvIndex(cbv.srvIndex),
		uavIndex(cbv.uavIndex),
		viewGPUHandle(cbv.viewGPUHandle),
		viewCPUHandle(cbv.viewCPUHandle),
		buffer(cbv.buffer ? makeUnique<D3D12Resource::Buffer>(*cbv.buffer) : nullptr)
	{
	}

	CounterBufferView::~CounterBufferView()
	{
	}

	ShaderResourceDesc CounterBufferView::getSRVIndex()
	{
		const ShaderResourceDesc desc = { .type = ShaderResourceDesc::BUFFER,
			                              .state = ShaderResourceDesc::SRV,
			                              .resourceIndex = srvIndex,
			                              .bufferDesc = { .buffer = buffer.get(),
			                                              .counterBuffer = nullptr } };

		return desc;
	}

	ShaderResourceDesc CounterBufferView::getUAVIndex()
	{
		const ShaderResourceDesc desc = { .type = ShaderResourceDesc::BUFFER,
			                              .state = ShaderResourceDesc::UAV,
			                              .resourceIndex = uavIndex,
			                              .bufferDesc = { .buffer = buffer.get(),
			                                              .counterBuffer = nullptr } };

		return desc;
	}

	UAVClearDesc CounterBufferView::getUAVClearDesc()
	{
		const UAVClearDesc desc = { .type = UAVClearDesc::BUFFER,
			                        .bufferDesc = { .buffer = buffer.get() },
			                        .viewGPUHandle = viewGPUHandle,
			                        .viewCPUHandle = viewCPUHandle };

		return desc;
	}

	D3D12Resource::Buffer* CounterBufferView::getBuffer() const
	{
		return buffer.get();
	}
}
