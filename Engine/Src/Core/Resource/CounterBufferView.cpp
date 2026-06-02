#include<Gear/Core/Resource/CounterBufferView.h>

Gear::Core::Resource::CounterBufferView::CounterBufferView(const bool persistent) :
	ResourceBase(persistent), buffer(new D3D12Resource::Buffer(4, true, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)), srvIndex(0), uavIndex(0), viewGPUHandle(), viewCPUHandle()
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

		srvIndex = descriptorHandle.getCurrentIndex();

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

		uavIndex = descriptorHandle.getCurrentIndex();

		if (persistent)
		{
			viewGPUHandle = descriptorHandle.getCurrentGPUHandle();

			const D3D12Core::DescriptorHandle nonShaderVisibleHandle = LocalDescriptorHeap::getStagingResourceHeap()->allocStaticDescriptor(1);

			GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), nullptr, &desc, nonShaderVisibleHandle.getCurrentCPUHandle());

			viewCPUHandle = nonShaderVisibleHandle.getCurrentCPUHandle();
		}
		else
		{
			viewCPUHandle = descriptorHandle.getCurrentCPUHandle();

			//之后获取viewGPUHandle
		}
	}
}

Gear::Core::Resource::CounterBufferView::~CounterBufferView()
{
	if (buffer)
	{
		delete buffer;
	}
}

Gear::Core::Resource::D3D12Resource::ShaderResourceDesc Gear::Core::Resource::CounterBufferView::getSRVIndex() const
{
	D3D12Resource::ShaderResourceDesc desc = {};
	desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
	desc.state = D3D12Resource::ShaderResourceDesc::SRV;
	desc.resourceIndex = srvIndex;
	desc.bufferDesc.buffer = buffer;

	return desc;
}

Gear::Core::Resource::D3D12Resource::ShaderResourceDesc Gear::Core::Resource::CounterBufferView::getUAVIndex() const
{
	D3D12Resource::ShaderResourceDesc desc = {};
	desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
	desc.state = D3D12Resource::ShaderResourceDesc::UAV;
	desc.resourceIndex = uavIndex;
	desc.bufferDesc.buffer = buffer;

	return desc;
}

Gear::Core::Resource::D3D12Resource::ClearUAVDesc Gear::Core::Resource::CounterBufferView::getClearUAVDesc() const
{
	D3D12Resource::ClearUAVDesc desc = {};
	desc.type = D3D12Resource::ClearUAVDesc::BUFFER;
	desc.bufferDesc.buffer = buffer;
	desc.viewGPUHandle = viewGPUHandle;
	desc.viewCPUHandle = viewCPUHandle;

	return desc;
}

void Gear::Core::Resource::CounterBufferView::copyDescriptors()
{
	D3D12Core::DescriptorHandle shaderVisibleHandle = copyToResourceHeap();

	srvIndex = shaderVisibleHandle.getCurrentIndex();

	shaderVisibleHandle.move();

	uavIndex = shaderVisibleHandle.getCurrentIndex();

	viewGPUHandle = shaderVisibleHandle.getCurrentGPUHandle();
}

Gear::Core::Resource::D3D12Resource::Buffer* Gear::Core::Resource::CounterBufferView::getBuffer() const
{
	return buffer;
}
