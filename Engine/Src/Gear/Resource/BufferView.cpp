#include <Gear/Resource/BufferView.h>

#include <Gear/Core/Graphics.h>

namespace Gear::Resource
{
	BufferView::BufferView(D3D12Resource::BufferPtr bufferPtr,
	                       const uint32_t structureByteStride,
	                       const DXGI_FORMAT format,
	                       const bool createSRV,
	                       const bool createUAV,
	                       const bool createVBV,
	                       const bool createIBV,
	                       const bool cpuWritable) :
		hasSRV(createSRV),
		hasUAV(createUAV),
		srvFormat(FMT::UNKNOWN),
		uavFormat(FMT::UNKNOWN),
		srvIndex(UINT32_MAX),
		uavIndex(UINT32_MAX),
		viewGPUHandle{},
		viewCPUHandle{},
		bufferViewUnion{},
		uploadHeaps(nullptr),
		buffer(std::move(bufferPtr)),
		counterBuffer(nullptr)
	{
		const uint32_t numGlobalResourceHeapDescriptors =
			static_cast<uint32_t>(createSRV) + static_cast<uint32_t>(createUAV);

		const bool isTypedBuffer = (structureByteStride == 0 && format != FMT::UNKNOWN);

		const bool isStructuredBuffer = (structureByteStride != 0 && format == FMT::UNKNOWN);

		const bool isByteAddressBuffer = (structureByteStride == 0 && format == FMT::UNKNOWN);

		if (numGlobalResourceHeapDescriptors)
		{
			//为结构化缓冲创建计数器缓冲
			if (isStructuredBuffer && createUAV)
			{
				counterBuffer = makeUnique<CounterBufferView>();
			}

			D3D12Core::DescriptorHandle* const descriptorHandle =
				createResourceDescriptorHandle(numGlobalResourceHeapDescriptors);

			if (createSRV)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
				desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

				if (isTypedBuffer)
				{
					srvFormat = format;

					desc.Format = format;
					desc.Buffer.NumElements =
						static_cast<uint32_t>(buffer->getSize()) / FMT::getByteSize(format);
				}
				else if (isStructuredBuffer)
				{
					desc.Buffer.NumElements = static_cast<uint32_t>(buffer->getSize()) / structureByteStride;
					desc.Buffer.StructureByteStride = structureByteStride;
				}
				else if (isByteAddressBuffer)
				{
					srvFormat = FMT::R32TL;

					desc.Format = FMT::R32TL;
					desc.Buffer.NumElements = static_cast<uint32_t>(buffer->getSize()) / 4;
					desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
				}

				GraphicsDevice::get()->CreateShaderResourceView(
					buffer->getResource(), &desc, descriptorHandle->getCurrentCPUHandle());

				srvIndex = descriptorHandle->getCurrentIndex();

				descriptorHandle->move();
			}

			if (createUAV)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

				if (isTypedBuffer)
				{
					uavFormat = format;

					desc.Format = format;
					desc.Buffer.NumElements =
						static_cast<uint32_t>(buffer->getSize()) / FMT::getByteSize(format);
				}
				else if (isStructuredBuffer)
				{
					desc.Buffer.NumElements = static_cast<uint32_t>(buffer->getSize()) / structureByteStride;
					desc.Buffer.StructureByteStride = structureByteStride;
				}
				else if (isByteAddressBuffer)
				{
					uavFormat = FMT::R32TL;

					desc.Format = FMT::R32TL;
					desc.Buffer.NumElements = static_cast<uint32_t>(buffer->getSize()) / 4;
					desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
				}

				if (isStructuredBuffer)
				{
					GraphicsDevice::get()->CreateUnorderedAccessView(
						buffer->getResource(),
						counterBuffer->getBuffer()->getResource(),
						&desc,
						descriptorHandle->getCurrentCPUHandle());
				}
				else
				{
					GraphicsDevice::get()->CreateUnorderedAccessView(
						buffer->getResource(), nullptr, &desc, descriptorHandle->getCurrentCPUHandle());
				}

				uavIndex = descriptorHandle->getCurrentIndex();

				viewGPUHandle = descriptorHandle->getCurrentGPUHandle();

				D3D12Core::DescriptorHandle* const nonShaderVisibleHandle = createNonShaderVisibleResourceDescriptorHandle(1);

				if (isStructuredBuffer)
				{
					GraphicsDevice::get()->CreateUnorderedAccessView(
						buffer->getResource(),
						counterBuffer->getBuffer()->getResource(),
						&desc,
						nonShaderVisibleHandle->getCurrentCPUHandle());
				}
				else
				{
					GraphicsDevice::get()->CreateUnorderedAccessView(
						buffer->getResource(), nullptr, &desc, nonShaderVisibleHandle->getCurrentCPUHandle());
				}

				viewCPUHandle = nonShaderVisibleHandle->getCurrentCPUHandle();
			}
		}

		if (createVBV)
		{
			bufferViewUnion.vbv.BufferLocation = buffer->getGPUAddress();
			bufferViewUnion.vbv.SizeInBytes = static_cast<uint32_t>(buffer->getSize());
			bufferViewUnion.vbv.StrideInBytes =
				(isStructuredBuffer ? structureByteStride : FMT::getByteSize(format));
		}

		if (createIBV)
		{
			bufferViewUnion.ibv.BufferLocation = buffer->getGPUAddress();
			bufferViewUnion.ibv.SizeInBytes = static_cast<uint32_t>(buffer->getSize());
			bufferViewUnion.ibv.Format = format;
		}

		if (cpuWritable)
		{
			uploadHeaps = makeUnique<UniquePtr<D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount());

			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				uploadHeaps[i] = makeUnique<D3D12Resource::UploadHeap>(buffer->getSize());
			}
		}
	}

	// TODO 待办 #1：拷贝构造没有重建 uploadHeaps，也没有自己的描述符句柄
	// 拷贝出来的对象调用 getUpdateStruct 会解引用空指针，句柄槽位也归源对象所有
#pragma message("TODO: BufferView 的拷贝构造尚未完成（uploadHeaps 为 nullptr / 描述符句柄与源对象共享）")
	BufferView::BufferView(const BufferView& bv) :
		ResourceBase(bv),
		hasSRV(bv.hasSRV),
		hasUAV(bv.hasUAV),
		srvFormat(bv.srvFormat),
		uavFormat(bv.uavFormat),
		srvIndex(bv.srvIndex),
		uavIndex(bv.uavIndex),
		viewGPUHandle(bv.viewGPUHandle),
		viewCPUHandle(bv.viewCPUHandle),
		bufferViewUnion(bv.bufferViewUnion),
		uploadHeaps(nullptr),
		buffer(bv.buffer ? makeUnique<D3D12Resource::Buffer>(*bv.buffer) : nullptr),
		counterBuffer(bv.counterBuffer ? makeUnique<CounterBufferView>(*bv.counterBuffer) : nullptr)
	{
	}

	BufferView::~BufferView()
	{
	}

	VertexBufferDesc BufferView::getVertexBuffer() const
	{
		const VertexBufferDesc desc = { .buffer = buffer.get(), .vbv = bufferViewUnion.vbv };

		return desc;
	}

	IndexBufferDesc BufferView::getIndexBuffer() const
	{
		const IndexBufferDesc desc = { .buffer = buffer.get(), .ibv = bufferViewUnion.ibv };

		return desc;
	}

	ShaderResourceDesc BufferView::getSRVIndex()
	{
		const ShaderResourceDesc desc = { .type = ShaderResourceDesc::BUFFER,
			                              .state = ShaderResourceDesc::SRV,
			                              .resource = this,
			                              .resourceIndex = &srvIndex,
			                              .bufferDesc = { .buffer = buffer.get(),
			                                              .counterBuffer = nullptr } };

		return desc;
	}

	ShaderResourceDesc BufferView::getUAVIndex()
	{
		const ShaderResourceDesc desc = {
			.type = ShaderResourceDesc::BUFFER,
			.state = ShaderResourceDesc::UAV,
			.resource = this,
			.resourceIndex = &uavIndex,
			.bufferDesc = { .buffer = buffer.get(),
			                .counterBuffer = (counterBuffer ? counterBuffer->getBuffer() : nullptr) }
		};

		return desc;
	}

	UAVClearDesc BufferView::getUAVClearDesc()
	{
		const UAVClearDesc desc = { .type = UAVClearDesc::BUFFER,
			                        .bufferDesc = { .buffer = buffer.get() },
			                        .viewGPUHandle = viewGPUHandle,
			                        .viewCPUHandle = viewCPUHandle };

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

	BufferView::UpdateStruct BufferView::getUpdateStruct(const void* const data, const uint64_t size)
	{
		uploadHeaps[Graphics::getFrameIndex()]->update(data, size);

		const UpdateStruct updateStruct = { buffer.get(), uploadHeaps[Graphics::getFrameIndex()].get() };

		return updateStruct;
	}

	BufferView::operator VertexBufferDesc() const
	{
		return getVertexBuffer();
	}

	BufferView::operator IndexBufferDesc() const
	{
		return getIndexBuffer();
	}
}
