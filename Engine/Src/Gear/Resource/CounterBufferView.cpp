#include<Gear/Resource/CounterBufferView.h>

#include<Gear/Core/FMT.h>

namespace Gear::Resource
{
	CounterBufferView::CounterBufferView(const bool persistent) :
		ResourceBase(persistent),
		srvIndex(makeShared<uint32_t>(0)),
		uavIndex(makeShared<uint32_t>(0)),
		viewGPUHandle(makeShared<D3D12_GPU_DESCRIPTOR_HANDLE>()),
		viewCPUHandle(makeShared<D3D12_CPU_DESCRIPTOR_HANDLE>()),
		buffer(makeUnique<D3D12Resource::Buffer>(sizeof(uint32_t), true, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
	{
		setNumCBVSRVUAVDescriptors(2);

		D3D12Core::DescriptorHandle descriptorHandle = allocCBVSRVUAVDescriptors();

		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Format = FMT::R32TL;
			desc.Buffer.FirstElement = 0;
			desc.Buffer.NumElements = 1;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

			GraphicsDevice::get()->CreateShaderResourceView(buffer->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

			*srvIndex = descriptorHandle.getCurrentIndex();

			descriptorHandle.move();
		}

		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			desc.Format = FMT::R32TL;
			desc.Buffer.FirstElement = 0;
			desc.Buffer.NumElements = 1;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

			GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), nullptr, &desc, descriptorHandle.getCurrentCPUHandle());

			*uavIndex = descriptorHandle.getCurrentIndex();

			if (persistent)
			{
				*viewGPUHandle = descriptorHandle.getCurrentGPUHandle();

				const D3D12Core::DescriptorHandle nonShaderVisibleHandle = LocalDescriptorHeap::getStagingResourceHeap()->allocStaticDescriptor(1);

				GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), nullptr, &desc, nonShaderVisibleHandle.getCurrentCPUHandle());

				*viewCPUHandle = nonShaderVisibleHandle.getCurrentCPUHandle();
			}
			else
			{
				*viewCPUHandle = descriptorHandle.getCurrentCPUHandle();

				//之后获取viewGPUHandle
			}
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

	ShaderResourceDesc CounterBufferView::getSRVIndex() const
	{
		const ShaderResourceDesc desc = {
		.type = ShaderResourceDesc::BUFFER,
		.state = ShaderResourceDesc::SRV,
		.resourceIndex = srvIndex.get(),
		.bufferDesc = {
				.buffer = buffer.get(),
				.counterBuffer = nullptr
		} };

		return desc;
	}

	ShaderResourceDesc CounterBufferView::getUAVIndex() const
	{
		const ShaderResourceDesc desc = {
		.type = ShaderResourceDesc::BUFFER,
		.state = ShaderResourceDesc::UAV,
		.resourceIndex = uavIndex.get(),
		.bufferDesc = {
				.buffer = buffer.get(),
				.counterBuffer = nullptr
		} };

		return desc;
	}

	UAVClearDesc CounterBufferView::getUAVClearDesc() const
	{
		const UAVClearDesc desc = {
		.type = UAVClearDesc::BUFFER,
		.bufferDesc = {
				.buffer = buffer.get()
		},
		.viewGPUHandle = *viewGPUHandle,
		.viewCPUHandle = *viewCPUHandle
		};

		return desc;
	}

	bool CounterBufferView::copyDescriptors()
	{
		D3D12Core::DescriptorHandle shaderVisibleHandle;

		const bool copied = copyToResourceHeap(shaderVisibleHandle);

		if (copied)
		{
			*srvIndex = shaderVisibleHandle.getCurrentIndex();

			shaderVisibleHandle.move();

			*uavIndex = shaderVisibleHandle.getCurrentIndex();

			*viewGPUHandle = shaderVisibleHandle.getCurrentGPUHandle();
		}

		return copied;
	}

	D3D12Resource::Buffer* CounterBufferView::getBuffer() const
	{
		return buffer.get();
	}
}