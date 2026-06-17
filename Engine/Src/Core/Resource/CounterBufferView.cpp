#include<Gear/Core/Resource/CounterBufferView.h>

namespace Gear::Core::Resource
{
	CounterBufferView::CounterBufferView(const bool persistent) :
		ResourceBase(persistent),
		srvIndex(makeShared<uint32_t>(0)),
		uavIndex(makeShared<uint32_t>(0)),
		viewGPUHandle(makeShared<D3D12_GPU_DESCRIPTOR_HANDLE>()),
		viewCPUHandle(makeShared<D3D12_CPU_DESCRIPTOR_HANDLE>()),
		buffer(makeUnique<D3D12Resource::Buffer>(4, true, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
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

	D3D12Resource::ShaderResourceDesc CounterBufferView::getSRVIndex() const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
		desc.state = D3D12Resource::ShaderResourceDesc::SRV;
		desc.resourceIndex = *srvIndex;
		desc.bufferDesc.buffer = buffer.get();

		return desc;
	}

	D3D12Resource::ShaderResourceDesc CounterBufferView::getUAVIndex() const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
		desc.state = D3D12Resource::ShaderResourceDesc::UAV;
		desc.resourceIndex = *uavIndex;
		desc.bufferDesc.buffer = buffer.get();

		return desc;
	}

	D3D12Resource::ClearUAVDesc CounterBufferView::getClearUAVDesc() const
	{
		D3D12Resource::ClearUAVDesc desc = {};
		desc.type = D3D12Resource::ClearUAVDesc::BUFFER;
		desc.bufferDesc.buffer = buffer.get();
		desc.viewGPUHandle = *viewGPUHandle;
		desc.viewCPUHandle = *viewCPUHandle;

		return desc;
	}

	void CounterBufferView::copyDescriptors()
	{
		D3D12Core::DescriptorHandle shaderVisibleHandle = copyToResourceHeap();

		*srvIndex = shaderVisibleHandle.getCurrentIndex();

		shaderVisibleHandle.move();

		*uavIndex = shaderVisibleHandle.getCurrentIndex();

		*viewGPUHandle = shaderVisibleHandle.getCurrentGPUHandle();
	}

	D3D12Resource::Buffer* CounterBufferView::getBuffer() const
	{
		return buffer.get();
	}
}