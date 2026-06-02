#include<Gear/Core/Resource/BufferView.h>

#include<Gear/Core/Graphics.h>

Gear::Core::Resource::BufferView::BufferView(D3D12Resource::Buffer* const buffer, const uint32_t structureByteStride, const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent) :
	ResourceBase(persistent), buffer(buffer), counterBuffer(nullptr), vbv{}, srvIndex(0), uavIndex(0), uploadHeaps(nullptr), hasSRV(createSRV), hasUAV(createUAV), viewCPUHandle(), viewGPUHandle()
{
	setNumCBVSRVUAVDescriptors(static_cast<uint32_t>(createSRV) + static_cast<uint32_t>(createUAV));

	const bool isTypedBuffer = (structureByteStride == 0 && format != FMT::UNKNOWN);

	const bool isStructuredBuffer = (structureByteStride != 0 && format == FMT::UNKNOWN);

	const bool isByteAddressBuffer = (structureByteStride == 0 && format == FMT::UNKNOWN);

	if (getNumCBVSRVUAVDescriptors())
	{
		//为结构化缓冲创建计数器缓冲
		if (isStructuredBuffer && createUAV)
		{
			counterBuffer = new CounterBufferView(persistent);
		}

		D3D12Core::DescriptorHandle descriptorHandle = allocCBVSRVUAVDescriptors();

		if (createSRV)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

			if (isTypedBuffer)
			{
				desc.Format = format;
				desc.Buffer.NumElements = static_cast<uint32_t>(size) / FMT::getByteSize(format);
			}
			else if (isStructuredBuffer)
			{
				desc.Buffer.NumElements = static_cast<uint32_t>(size) / structureByteStride;
				desc.Buffer.StructureByteStride = structureByteStride;
			}
			else if (isByteAddressBuffer)
			{
				desc.Format = FMT::R32TL;
				desc.Buffer.NumElements = static_cast<uint32_t>(size) / 4;
				desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
			}

			GraphicsDevice::get()->CreateShaderResourceView(buffer->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

			srvIndex = descriptorHandle.getCurrentIndex();

			descriptorHandle.move();
		}

		if (createUAV)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

			if (isTypedBuffer)
			{
				desc.Format = format;
				desc.Buffer.NumElements = static_cast<uint32_t>(size) / FMT::getByteSize(format);
			}
			else if (isStructuredBuffer)
			{
				desc.Buffer.NumElements = static_cast<uint32_t>(size) / structureByteStride;
				desc.Buffer.StructureByteStride = structureByteStride;
			}
			else if (isByteAddressBuffer)
			{
				desc.Format = FMT::R32TL;
				desc.Buffer.NumElements = static_cast<uint32_t>(size) / 4;
				desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
			}

			if (isStructuredBuffer)
			{
				GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), counterBuffer->getBuffer()->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());
			}
			else
			{
				GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), nullptr, &desc, descriptorHandle.getCurrentCPUHandle());
			}

			uavIndex = descriptorHandle.getCurrentIndex();

			//对于持久性资源我们需要在非着色器可见的描述符堆创建一个额外的描述符来获得一个viewCPUHandle
			//而对于非持久性资源，我们已经有了
			if (persistent)
			{
				viewGPUHandle = descriptorHandle.getCurrentGPUHandle();

				const D3D12Core::DescriptorHandle nonShaderVisibleHandle = LocalDescriptorHeap::getStagingResourceHeap()->allocStaticDescriptor(1);

				if (isStructuredBuffer)
				{
					GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), counterBuffer->getBuffer()->getResource(), &desc, nonShaderVisibleHandle.getCurrentCPUHandle());
				}
				else
				{
					GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), nullptr, &desc, nonShaderVisibleHandle.getCurrentCPUHandle());
				}

				viewCPUHandle = nonShaderVisibleHandle.getCurrentCPUHandle();
			}
			else
			{
				viewCPUHandle = descriptorHandle.getCurrentCPUHandle();

				//之后获取viewGPUHandle
			}
		}
	}

	if (createVBV)
	{
		vbv.BufferLocation = buffer->getGPUAddress();
		vbv.SizeInBytes = static_cast<uint32_t>(size);
		vbv.StrideInBytes = (isStructuredBuffer ? structureByteStride : FMT::getByteSize(format));
	}

	if (createIBV)
	{
		ibv.BufferLocation = buffer->getGPUAddress();
		ibv.SizeInBytes = static_cast<uint32_t>(size);
		ibv.Format = format;
	}

	if (cpuWritable)
	{
		uploadHeaps = new D3D12Resource::UploadHeap * [Graphics::getFrameBufferCount()];

		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			uploadHeaps[i] = new D3D12Resource::UploadHeap(size);
		}
	}
}

Gear::Core::Resource::BufferView::~BufferView()
{
	if (counterBuffer)
	{
		delete counterBuffer;
	}

	if (buffer)
	{
		delete buffer;
	}

	if (uploadHeaps)
	{
		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			delete uploadHeaps[i];
		}

		delete[] uploadHeaps;
	}
}

Gear::Core::Resource::D3D12Resource::VertexBufferDesc Gear::Core::Resource::BufferView::getVertexBuffer() const
{
	D3D12Resource::VertexBufferDesc desc = {};
	desc.buffer = buffer;
	desc.vbv = vbv;

	return desc;
}

Gear::Core::Resource::D3D12Resource::IndexBufferDesc Gear::Core::Resource::BufferView::getIndexBuffer() const
{
	D3D12Resource::IndexBufferDesc desc = {};
	desc.buffer = buffer;
	desc.ibv = ibv;

	return desc;
}


Gear::Core::Resource::D3D12Resource::ShaderResourceDesc Gear::Core::Resource::BufferView::getSRVIndex() const
{
	D3D12Resource::ShaderResourceDesc desc = {};
	desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
	desc.state = D3D12Resource::ShaderResourceDesc::SRV;
	desc.resourceIndex = srvIndex;
	desc.bufferDesc.buffer = buffer;

	return desc;
}

Gear::Core::Resource::D3D12Resource::ShaderResourceDesc Gear::Core::Resource::BufferView::getUAVIndex() const
{
	D3D12Resource::ShaderResourceDesc desc = {};
	desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
	desc.state = D3D12Resource::ShaderResourceDesc::UAV;
	desc.resourceIndex = uavIndex;
	desc.bufferDesc.buffer = buffer;
	desc.bufferDesc.counterBuffer = (counterBuffer ? counterBuffer->getBuffer() : nullptr);

	return desc;
}

Gear::Core::Resource::D3D12Resource::ClearUAVDesc Gear::Core::Resource::BufferView::getClearUAVDesc() const
{
	D3D12Resource::ClearUAVDesc desc = {};
	desc.type = D3D12Resource::ClearUAVDesc::BUFFER;
	desc.bufferDesc.buffer = buffer;
	desc.viewGPUHandle = viewGPUHandle;
	desc.viewCPUHandle = viewCPUHandle;

	return desc;
}

Gear::Core::Resource::CounterBufferView* Gear::Core::Resource::BufferView::getCounterBuffer() const
{
	return counterBuffer;
}

Gear::Core::Resource::D3D12Resource::Buffer* Gear::Core::Resource::BufferView::getBuffer() const
{
	return buffer;
}

void Gear::Core::Resource::BufferView::copyDescriptors()
{
	D3D12Core::DescriptorHandle shaderVisibleHandle = copyToResourceHeap();

	if (hasSRV)
	{
		srvIndex = shaderVisibleHandle.getCurrentIndex();

		shaderVisibleHandle.move();
	}

	if (hasUAV)
	{
		uavIndex = shaderVisibleHandle.getCurrentIndex();

		viewGPUHandle = shaderVisibleHandle.getCurrentGPUHandle();
	}
}

Gear::Core::Resource::BufferView::UpdateStruct Gear::Core::Resource::BufferView::getUpdateStruct(const void* const data, const uint64_t size)
{
	uploadHeaps[Graphics::getFrameIndex()]->update(data, size);

	const UpdateStruct updateStruct = { buffer,uploadHeaps[Graphics::getFrameIndex()] };

	return updateStruct;
}
