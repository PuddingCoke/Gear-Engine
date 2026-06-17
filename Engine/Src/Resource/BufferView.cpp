#include<Gear/Resource/BufferView.h>

#include<Gear/Core/Graphics.h>

namespace Gear::Resource
{
	BufferView::BufferView(D3D12Resource::BufferPtr bufferPtr, const uint32_t structureByteStride, const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent) :
		ResourceBase(persistent),
		hasSRV(createSRV),
		hasUAV(createUAV),
		srvFormat(FMT::UNKNOWN),
		uavFormat(FMT::UNKNOWN),
		counterBuffer(nullptr),
		srvIndex(makeShared<uint32_t>()),
		uavIndex(makeShared<uint32_t>()),
		viewGPUHandle(makeShared<D3D12_GPU_DESCRIPTOR_HANDLE>()),
		viewCPUHandle(makeShared<D3D12_CPU_DESCRIPTOR_HANDLE>()),
		bufferViewStruct(makeShared<BufferViewStruct>()),
		uploadHeaps(nullptr),
		buffer(std::move(bufferPtr))
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
				counterBuffer = makeUnique<CounterBufferView>(persistent);
			}

			D3D12Core::DescriptorHandle descriptorHandle = allocCBVSRVUAVDescriptors();

			if (createSRV)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
				desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

				if (isTypedBuffer)
				{
					srvFormat = format;

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
					srvFormat = FMT::R32TL;

					desc.Format = FMT::R32TL;
					desc.Buffer.NumElements = static_cast<uint32_t>(size) / 4;
					desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
				}

				GraphicsDevice::get()->CreateShaderResourceView(buffer->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

				*srvIndex = descriptorHandle.getCurrentIndex();

				descriptorHandle.move();
			}

			if (createUAV)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

				if (isTypedBuffer)
				{
					uavFormat = format;

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
					uavFormat = FMT::R32TL;

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

				*uavIndex = descriptorHandle.getCurrentIndex();

				//对于持久性资源我们需要在非着色器可见的描述符堆创建一个额外的描述符来获得一个viewCPUHandle
				//而对于非持久性资源，我们已经有了
				if (persistent)
				{
					*viewGPUHandle = descriptorHandle.getCurrentGPUHandle();

					const D3D12Core::DescriptorHandle nonShaderVisibleHandle = LocalDescriptorHeap::getStagingResourceHeap()->allocStaticDescriptor(1);

					if (isStructuredBuffer)
					{
						GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), counterBuffer->getBuffer()->getResource(), &desc, nonShaderVisibleHandle.getCurrentCPUHandle());
					}
					else
					{
						GraphicsDevice::get()->CreateUnorderedAccessView(buffer->getResource(), nullptr, &desc, nonShaderVisibleHandle.getCurrentCPUHandle());
					}

					*viewCPUHandle = nonShaderVisibleHandle.getCurrentCPUHandle();
				}
				else
				{
					*viewCPUHandle = descriptorHandle.getCurrentCPUHandle();

					//之后获取viewGPUHandle
				}
			}
		}

		if (createVBV)
		{
			bufferViewStruct->vbv.BufferLocation = buffer->getGPUAddress();
			bufferViewStruct->vbv.SizeInBytes = static_cast<uint32_t>(size);
			bufferViewStruct->vbv.StrideInBytes = (isStructuredBuffer ? structureByteStride : FMT::getByteSize(format));
		}

		if (createIBV)
		{
			bufferViewStruct->ibv.BufferLocation = buffer->getGPUAddress();
			bufferViewStruct->ibv.SizeInBytes = static_cast<uint32_t>(size);
			bufferViewStruct->ibv.Format = format;
		}

		if (cpuWritable)
		{
			uploadHeaps = makeUnique<UniquePtr<D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount());

			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				uploadHeaps[i] = makeUnique<D3D12Resource::UploadHeap>(size);
			}
		}
	}

	BufferView::BufferView(const BufferView& bv) :
		ResourceBase(bv),
		hasSRV(bv.hasSRV),
		hasUAV(bv.hasUAV),
		srvFormat(bv.srvFormat),
		uavFormat(bv.uavFormat),
		counterBuffer(bv.counterBuffer ? makeUnique<CounterBufferView>(*bv.counterBuffer) : nullptr),
		srvIndex(bv.srvIndex),
		uavIndex(bv.uavIndex),
		viewGPUHandle(bv.viewGPUHandle),
		viewCPUHandle(bv.viewCPUHandle),
		bufferViewStruct(bv.bufferViewStruct),
		uploadHeaps(nullptr),
		buffer(bv.buffer ? makeUnique<D3D12Resource::Buffer>(*bv.buffer) : nullptr)
	{
	}

	BufferView::~BufferView()
	{
	}

	D3D12Resource::VertexBufferDesc BufferView::getVertexBuffer() const
	{
		D3D12Resource::VertexBufferDesc desc = {};
		desc.buffer = buffer.get();
		desc.vbv = bufferViewStruct->vbv;

		return desc;
	}

	D3D12Resource::IndexBufferDesc BufferView::getIndexBuffer() const
	{
		D3D12Resource::IndexBufferDesc desc = {};
		desc.buffer = buffer.get();
		desc.ibv = bufferViewStruct->ibv;

		return desc;
	}


	D3D12Resource::ShaderResourceDesc BufferView::getSRVIndex() const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
		desc.state = D3D12Resource::ShaderResourceDesc::SRV;
		desc.resourceIndex = *srvIndex;
		desc.bufferDesc.buffer = buffer.get();

		return desc;
	}

	D3D12Resource::ShaderResourceDesc BufferView::getUAVIndex() const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
		desc.state = D3D12Resource::ShaderResourceDesc::UAV;
		desc.resourceIndex = *uavIndex;
		desc.bufferDesc.buffer = buffer.get();
		desc.bufferDesc.counterBuffer = (counterBuffer.get() ? counterBuffer->getBuffer() : nullptr);

		return desc;
	}

	D3D12Resource::ClearUAVDesc BufferView::getClearUAVDesc() const
	{
		D3D12Resource::ClearUAVDesc desc = {};
		desc.type = D3D12Resource::ClearUAVDesc::BUFFER;
		desc.bufferDesc.buffer = buffer.get();
		desc.viewGPUHandle = *viewGPUHandle;
		desc.viewCPUHandle = *viewCPUHandle;

		return desc;
	}

	CounterBufferView* BufferView::getCounterBuffer() const
	{
		return counterBuffer.get();
	}

	D3D12Resource::Buffer* BufferView::getBuffer() const
	{
		return buffer.get();
	}

	void BufferView::copyDescriptors()
	{
		D3D12Core::DescriptorHandle shaderVisibleHandle = copyToResourceHeap();

		if (hasSRV)
		{
			*srvIndex = shaderVisibleHandle.getCurrentIndex();

			shaderVisibleHandle.move();
		}

		if (hasUAV)
		{
			*uavIndex = shaderVisibleHandle.getCurrentIndex();

			*viewGPUHandle = shaderVisibleHandle.getCurrentGPUHandle();
		}
	}

	BufferView::UpdateStruct BufferView::getUpdateStruct(const void* const data, const uint64_t size)
	{
		uploadHeaps[Graphics::getFrameIndex()]->update(data, size);

		const UpdateStruct updateStruct = { buffer.get(),uploadHeaps[Graphics::getFrameIndex()].get() };

		return updateStruct;
	}
}
